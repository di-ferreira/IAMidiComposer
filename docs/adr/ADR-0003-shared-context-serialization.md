# 0003. Serialização do Shared Musical Context — proto vs JSON

Data: 2026-07-10
Status: Proposta

## Contexto

O Shared Musical Context (SMC) é o estado compartilhado de cada projeto do AI
MIDI Composer — armazena tempo, tom, escala, estrutura, progressão, instrumentos,
histórico, preferências e regiões bloqueadas (ver `CONTEXT.md` → "Shared Musical
Context" e `docs/roadmap/ROADMAP.md` Fase 1.3). O SMC tem três papéis distintos
no sistema:

1. **Persistência em SQLite** — gravado e lido pela camada de Repository em Go
   (Fase 1.3 do roadmap), sob o schema `shared_context`, `sections`, `instruments`,
   `history`, `locks`.
2. **Tráfego no boundary gRPC entre ACE (Go) e MTE (C++)** — cruza o boundary
   definido no ADR-0001 a cada chamada `GenerateComposition` e respostas correlatas.
3. **Tráfego entre ACE e Plugin VST3** — o plugin lê/atualiza partes do SMC
   via gRPC para ACE (nunca direto com o MTE, respeitando §3.1 do master prompt).

Os tipos já declarados em `proto/aimidi/v1/aimidi.proto` cobrem o núcleo do SMC:
`MusicBlueprint`, `Energy`, `Mood`, `Timeline`, `Section`, `Instrument`,
`InstrumentRole`, `MicroEvent` — todos já versionados sob o path `/aimidi.v1`.

Existem dois candidatos principais para serialização do SMC:

- **(i) Protobuf (textual ou binária)** como formato único em todas as bordas e
  na persistência. Em SQLite, a coluna seria `BLOB` (binária proto) ou `TEXT`
  (proto textual / base64 da binária).
- **(ii) JSON** na persistência (humano-editável) e proto apenas no wire gRPC.
  Exigiria conversão explícita proto ↔ JSON via `protojson` (lado Go) e
  `protobuf/json` (lado C++ grpc-cpp skill) em cada borda.

Forças em conflito que motivam esta decisão:

- **Ler e editar SQLite manualmente** (DevTools do navegador, `sqlite3` CLI,
  DB Browser for SQLite) favorece JSON — um DBA ou contribuidor Open Source
  consegue inspecionar e corrigir um contexto corrompido sem código.
- **Performance e evolução de schema com tipos versionados** favorecem proto —
  parse de binária é uma ordem de magnitude mais rápido e o schema é estrito.
- **Não duplicar schemas**: sempre que alteramos `proto/aimidi/v1/aimidi.proto`,
  não queremos manter um segundo schema paralelo em structs Go ou C++ manuais
  só para suportar JSON na persistência. Essa duplicação é fonteWell-known de
  drift silencioso e bugs de versionamento.

## Decisao

Adotar **protobuf como schema canônico único** e serialização **binária
(bytes) em SQLite `BLOB`** para o Shared Musical Context. Detalhes:

1. **Schema canônico.** O único contrato de tipos do SMC é
   `proto/aimidi/v1/aimidi.proto`. Qualquer struct Go/C++ que precise manipular
   o contexto deriva dos tipos gerados a partir do proto (via `protoc-gen-go`
   e `grpc-cpp`). Não há structs manuais paralelas para o SMC.

2. **Coluna persistida.** O schema SQLite usa:

   ```sql
   CREATE TABLE shared_context (
       id           TEXT PRIMARY KEY,
       payload      BLOB NOT NULL,
       payload_type TEXT NOT NULL DEFAULT 'application/vnd.aimidi.context.v1+protobuf',
       -- ...demais colunas de auditoria: created_at, updated_at, seed, project_id...
   );
   ```

   `payload` guarda os bytes proto serializados. `payload_type` é um
   MIME-ish versionado (`application/vnd.aimidi.context.v1+protobuf`) que
   permite future migration de versão sem ambiguidade.

3. **Debugging manual via CLI/API.** Para inspeção humana, expor um subcomando:

   ```
   aimidi context export <id> --json
   ```

   que converte `bytes proto → JSON` via `protojson` (Go). A coluna persistida
   permanece **BLOB binária** — o JSON é só uma projeção de leitura, nunca a
   fonte da verdade. Editar o JSON e gravar de volta é out of scope desta fase;
   se solicitado futuramente, vira um `aimidi context import --json` que
   re-serializa para proto antes de persistir.

4. **Dumps em logs.** Para `spdlog` (C++) e `slog` (Go), tracear um _sumário_
   textual compacto no formato:

   ```
   <context id=… barras=X key=C energy=high mood=upbeat instruments=5>
   ```

   Nunca o JSON completo — evita ruído / PII nos logs e mantém as linhas
   legíveis. O sumário é derivado dos campos-chave do proto, não re-serializa.

5. **Migrations futuras.** Quando houver quebra de compatibilidade de schema
   (ex.: transição `v1` → `v2`), rodar uma função
   `proto::Migrate(bytes, from_v, to_v)` em Go **antes** de devolver o payload
   para ACE/MTE. **Nunca mutar o BLOB antigo inline** — a migração é preguiçosa
   (lazy) ou em batch, e a coluna `payload_type` registra a versão atual. Isso
   garante reproducibility de seeds mesmo após mudanças de schema.

## Alternativas consideradas

- **A. JSON como fonte da verdade; proto só no wire gRPC.**
  - Vantagens: humanos editam SQLite manualmente (ótimo para DBA / debugging
    sem ferramenta); nenhuma conversão na persistência; ferramentas de diff
    legíveis em git (se alguém commitar um contexto de exemplo).
  - Por que não:
    (i) precisamos manter dois schemas sincronizados (proto + texto JSON),
      duplicando tipos;
    (ii) tipos float de `Energy`/VAD sujeitos a diferenças de parsing entre
      bibliotecasGo e C++ (ex.: `1.0` vs `1`, precisão, notação científica);
    (iii) sem validação estrita de schema — um campo obrigatório ausente só
      vira erro em runtime, não em compile/bluild;
    (iv) análise de diferença de versão complexa (não há field numbers proto
      para considerar "desconhecido" vs "ausente").

- **B. Protobuf binária pura sem `payload_type` (sem MIME).**
  - Vantagens: schema mínimo, uma coluna a menos.
  - Por que não: sem `payload_type`, a migração de versão futura exige
    heurística frágil (tamanho do BLOB? magic bytes? parsing de tentativa?)
    para distinguir `v1` de `v2`. Esse problema piora quando rodamos migrations
    frequentes na Fase 4 (expansão horizontal do MTE). O custo de uma coluna
    extra de texto é desprezível e elimina ambiguidade estrutural.

- **C. JSON binário (MessagePack).**
  - Vantagens: compacto, schemaless, mais rápido que JSON textual.
  - Por que não: ainda sem validação estrita; não resolve a duplicação de tipo
    proto (continuaríamos precisando de um schema paralelo); adiciona mais uma
    dependência (`msgp` / `msgpack-go`) ao build — viola o princípio de mínimas
    dependências externas. O ganho de tamanho sobre proto binária é marginal
    para payloads de poucos KB.

- **D. SQLite JSON1 + proto binário (híbrido).**
  - Vantagens: permite queries SQL sobre campos JSON extraídos via funções
    `json_extract()` do SQLite.
  - Por que não: a extensão JSON1 não roda em todos os builds embedded
    (especialmente builds estáticos de SQLite em paths Windows/macOS where
    the amalgamation é compilada sem ela); a coluna BLOB persistida foi escolhida
    por simplicidade. Queries estruturadas sobre o SMC são raras — o acesso é
    quase sempre por `id` (PK), não por campos internos do payload.

## Consequencias

- **Positivas:**
  - (a) **Schema único canônico** — `proto/aimidi/v1/aimidi.proto` é a única
    fonte de tipos; elimina drift de schemas entre Go e C++.
  - (b) **Performance** — runtime de parse de proto binária é da ordem de
    microssegundos contra centenas de microssegundos de JSON textual para o
    mesmo payload. Relevante porque o SMC cruza o boundary gRPC a cada request.
  - (c) **Validação estrita** pelo proto — campos obrigatórios ausentes viram
    erro de parse, não `null` silencioso.
  - (d) **MIME versionado** (`payload_type`) habilita migration futura sem
    ambiguidade — alinha-se ao princípio de seed-reproducibility através de
    versões do schema.
  - (e) **Tamanho neutro** — SQLite típico não diferencia BLOB de TEXT em
    custo de armazenamento para poucos KB de contexto; o ganho de schema
    estrito compensa qualquer diferença marginal.

- **Negativas:**
  - (a) **Inspeção manual direta no SQLite fica opaca** — um `SELECT payload
    FROM shared_context` devolve bytes binários ilegíveis. Mitigada pelo CLI
    `aimidi context export <id> --json`, que cobre o caso de debugging humano.
  - (b) **`protojson` (Go) e `protobuf/json` (C++) viram dependências a mais**
    — mas ambas já estão implícitas na stack (gRPC/protobuf já é dependência
    principal definida no ADR-0001 e na stack tecnológica de `CONTEXT.md`).
    Não é uma nova dependência, apenas uma extensão de uso das já presentes.
  - (c) **Diff de binário proto é ilegível em git** — o SMC não deve ser
    commitado (é runtime state), então isso é aceitável. Se oportunamente
    quiseremos fixtures de teste em proto, usaremos proto text format
    (`.textproto`), que é legível e diff-friendly.

- **Neutras:**
  - (a) Coincide com o `CompositionResponse.shared_context_id` (string) já
    declarado no proto — o payload BLOB vive em SQLite indexado por esse `id`.
    O campo proto pronto, sem precisar de novo design.
  - (b) Se futuramente usuários pedirem import/export `.aimidi` (JSON), o CLI
    `--json` já cobre o caso de exportação; a importação seguiria o mesmo caminho
    inverso (`JSON → proto → BLOB`).
  - (c) Na Fase 7.3, a AI model distribution (ADR-0006) usará o mesmo padrão de
    MIME versionado — mantém consistência de versionamento entre SMC e modelos
    de IA locais.

## Relacionado

- ADR-0001 — Boundary C++ ↔ Go. O SMC cruza esse boundary em cada chamada gRPC;
  a serialização proto binária aqui definida é o formato no wire.
- ADR-0002 — DI container C++20. O Repository Go que persiste o SMC é injetado
  no lado Go; os tipos proto do SMC são resolvidos no lado C++ via `ServiceLocator`.
- Skills: `protobuf-basics`, `sqlite-go`, `grpc-cpp` (para serialização proto
  no lado C++). Futuramente: `database-migrations` (quando a migration `v1 → v2`
  for necessária).
- Standards: `standards/architecture.md` (§4 — comunicação entre módulos),
  `standards/security.md` (tamanho de logs / PII).
- Roadmap: ver `docs/roadmap/ROADMAP.md` Fase 1.3 — "Schema SQLite
  (`shared_context`, `sections`, `instruments`, `history`, `locks`) + ADR-0003".
- Código: `proto/aimidi/v1/aimidi.proto` (mensagens já declaradas, `MusicBlueprint`,
  `Energy`, `Mood`, `Timeline`, `Section`, `Instrument`, `InstrumentRole`,
  `MicroEvent`).
