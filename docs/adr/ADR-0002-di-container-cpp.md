# 0002. Dependency Injection container no MTE C++20

Data: 2026-07-10
Status: Proposta

## Contexto

O Music Theory Engine (MTE) em C++ já possui duas interfaces virtuais puras com
factory functions gratuitas — `IScaleProvider` (`engine/cpp/include/aimidi/theory/IScaleProvider.hpp:14`)
expõe `make_scale_provider()` e `IHarmonyEngine`
(`engine/cpp/include/aimidi/theory/IHarmonyEngine.hpp:24`) expõe
`make_harmony_engine(std::shared_ptr<IScaleProvider>)`. Esse par é, na prática,
um embrião de Dependency Injection (DI) manual: o `HarmonyEngine` recebe como
construtor argumento o `IScaleProvider` do qual depende, e a factory pública
encapsula a montagem.

Com o avanço da Fase 1.1 do roadmap (milestone M1), o MTE crescerá para cerca de
13 sub-engines expostas como interfaces `I*.hpp` sob `engine/cpp/include/aimidi/<domain>/`:
`IChordEngine`, `IRhythmEngine`, `IBassEngine`, `IDrumEngine`, `IMelodyEngine`,
`IGuitarEngine`, `IPianoEngine`, `IStringsEngine`, `IOrchestrationEngine`,
`IHumanizationEngine`, `IMidiRenderer`, `IInstrumentMapper`, além das duas já
existentes. O grafo de dependências não é trivial:

- `IChordEngine` precisa de `IHarmonyEngine` + `IScaleProvider`.
- `IBassEngine` precisa de `IHarmonyEngine` + `IRhythmEngine`.
- `IStringsEngine` precisa de `IHarmonyEngine` + `IRhythmEngine` + `IOrchestrationEngine`.
- E assim por diante.

A estratégia atual (factories manuais espalhadas pelos call sites) começa a
apresentar sintomas claros de degradação:

1. **Explosão combinatória de wiring** — cada caller (binário de produção,
   suíte GoogleTest, binário de benchmark) precisa repetir a árvore de montage na
   ordem correta; uma sub-engine nova significa editar todos os callers.
2. **Fértil para bugs e duplicação** — ordem de construção fica acoplada ao
   call site, e erros sutis (esquecer uma dependência, trocar a ordem) surgem em
   runtime, não em compile time.
3. **Violação de DRY** — a topologia do grafo de services é reescrita N vezes.

Precisamos de um padrão unificado de DI para o lado C++/MTE que respeite:

- (a) preserve Dependency Injection e o princípio SOLID de Inversion of Control
  (§7 do `prompts/master_prompt.md` e `standards/architecture.md` §1).
- (b) seja header-only ou trivial — 100% offline, cross-platform, sem download
  de pacotes externos em build time.
- (c) permita testes com mocks (GoogleTest) sem acrobacias.
- (d) seja idiomático C++20, sem macros-exóticas nem reflection mágica não-padrão.
- (e) não introduza dependências pesadas externas — `boost::di` é header-only,
  mas traz aproximadamente 5.000 linhas de templates densos, curva de
  aprendizado para contributors Open Source e impacto mensurável no tempo de
  compilação do MTE completo.

## Decisao

Adotar um **`ServiceLocator`** pequeno e explícito em
`engine/cpp/include/aimidi/core/ServiceLocator.hpp` — um container de DI mínimo,
cabe em poucas dezenas de linhas, exposto como:

```cpp
template <class I>
auto resolve() -> std::shared_ptr<I>;
```

Apoia-se internamente em `std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>>`
preenchido por funções explícitas e idempotentes, uma por módulo, por exemplo
`register_music_theory(ServiceLocator&)`.

**Não adotar `boost::di` nesta fase.** A escolha será revisitada na Fase 4
do roadmap se a complexidade do grafo justificar.

Regras de uso, válidas para todo o MTE:

1. **Um `register_*` por módulo musical.** O módulo `music-theory` fornece
   `register_music_theory(ServiceLocator&)` que registra `IScaleProvider`,
   `IHarmonyEngine`, `IChordEngine`, etc. Cada `register_*` é
   **idempotente** (segunda chamada sobrescreve o binding anterior, nunca
   duplica) e **topologicamente ordenado** — resolve suas próprias dependências
   internas chamando `resolve<Dep>()` após tê-las registrado. O `ServiceLocator`
   **não faz auto-wiring**: quem registra é responsável pela ordem.

2. **Em testes (GoogleTest)**, cada fixture constrói um `ServiceLocator` local
   no heap da fixture e registra apenas as implementações (ou mocks) necessárias
   para aquele teste — respeitando o princípio de隔离 (isolation) do gtest e o
   paralelismo de testes. Não há estado global compartilhado entre casos de teste.
   Os mocks seguem o padrão ` MOCK_METHOD` sobre as interfaces `I*.hpp`.

3. **Em produção**, `ServiceLocator::production()` constrói o grafo default
   completo, chamando cada `register_*` na ordem de módulos, e devolve uma
   instância pronta para `resolve<T>()`.

4. **Ownership via `std::shared_ptr<T>`.** `resolve<T>()` retorna
   `std::shared_ptr<T>` (jamais `T*` bruto) para ownership claro e seguro em
   contextos multi-thread sem estado compartilhado mutável entre requests. Se
   um binding estiver faltante, `resolve<T>()` lança
   `std::runtime_error("unregistered: <demangled>")` — usando `boost::core::demangle`
   ou equivalente disponível no toolchain.

5. **Lifetime: todos shared.** Aceitável para o MTE porque suas engines são
   stateless ou reentrantes entre requests. **Estado por request** (seed, cache
   arena, buffers) vive no `RequestContext` passado como argumento aos métodos
   das engines — nunca no `ServiceLocator`. Essa separação é obrigatória e
   garante que o container seja thread-safe para leitura concorrente.

6. **Header-only, sem deps externas.** `ServiceLocator.hpp` depende apenas da
   biblioteca padrão C++20 (`<memory>`, `<typeindex>`, `<functional>`,
   `<unordered_map>`, `<stdexcept>`).

## Alternativas consideradas

- **A. `boost::di` (header-only).**
  - Vantagens: resolução automática de construtores (auto-wiring), constructor
    injection puro, bindings declarativos, escopo de lifetime (singleton /
    transient / request) embutido. É a escolha "canônica" de DI em C++ moderno.
  - Por que não nesta fase:
    (i) dependência de header template muito denso (~5k linhas) — aumenta
    tempo de compilação do MTE completo, que é o hot path crítico do projeto;
    (ii) binário final cresce por causa de instanciações de template exóticas
    (impacto mensurável em builds de release);
    (iii) curva de aprendizado para contributors Open Source que não conhecem
    a DSL do `boost::di` (`make_injector`, `bind<>.to<>()`, annotations);
    (iv) o ganho de auto-wiring não justifica agora — o grafo do MTE ainda é
    raso (profundidade 2–3) e tem ~13 nós. Re-avaliar na Fase 4 se algum
    `register_*` ultrapassar ~100 linhas de boilerplate de wiring manual.

- **B. DI manual com factories espalhadas (status quo).**
  - Exemplo: `make_chord_engine(make_harmony_engine(make_scale_provider()), make_rhythm_engine())`.
  - Por que não: explosão combinatória já descrita; ordem de chamada acoplada
    ao call site; replicação em cada caller (`main` + suíte de testes + binário
    de benchmark); viola DRY e dificulta refator concorrente de múltiplos
    engineers mexendo no mesmo grafo. É exatamente o problema que esta ADR visa
    resolver.

- **C. `ServiceLocator` como variável global (estilo singleton clássico).**
  - Idêntica à Decisão em implementação, mas com a instância vivendo numa
    variável de escopo global em vez de ser injetada.
  - Por que não: polui o estado global entre testes (um teste que registra um
    mock afeta o próximo); quebra o paralelismo de testes do GoogleTest
    (`--gtest_parallel`); viola Dependency Inversion / Inversion of Control
    porque o caller obtém a dependência puxando de um global em vez de recebê-la.
    É o antipadrão clássico de ServiceLocator — esta ADR evita-o exigindo que
    a instância seja explicitamente injetada.

- **D. Policy-based templates (`template template parameters`).**
  - Projeta dependências como templates em compile time, eliminando virtual
    dispatch e enable de inlining total. Padrão comum em bibliotecas de áudio
    de alta performance.
  - Por que não: aumenta drasticamente o tempo de compilação para o MTE
    completo (13 engines × políticas); reduz ergonomia de mocks em testes
    (cada política precisa de um fake compilável); acopla o caller aos tipos
    concretos. **Store-shelved** para futuras hot paths críticas — bench targets
    na Fase 7.2 (`docs/roadmap/ROADMAP.md` Fase 7.2) podem promover policy-based
    em engines isoladas se o profiling justificar. Não é apropriado para o
    pipeline principal de composição nesta fase, onde a flexibilidade de mock
    e aclareza de DI superam o ganho de virtual-call elision.

## Consequencias

- **Positivas:**
  - (a) **Zero dependência externa** — `ServiceLocator.hpp` usa só a stdlib C++20,
    alinhado ao princípio "100% offline, sem download em build".
  - (b) **Explícito e idiomático** — o grafo de wiring é visível em uma função
    por módulo, facilita onboarding de contributors.
  - (c) **Fácil de mockar em GoogleTest** — cada fixture constrói seu próprio
    container e registra apenas o necessário, com `MOCK_METHOD` sobre as interfaces.
  - (d) **DRY respeitado** — a topologia do grafo é declarada em um lugar por
    módulo (`register_*`), reduzindo a replicação atual em callers.
  - (e) **Build incremental async/parallel** — cada `register_*` é compilável
    de forma independente; recompilar um módulo não invalida os outros.
  - (f) **Ownership via `shared_ptr`** é seguro para multi-thread sem estado
    mutável compartilhado, respeitando a regra inviolável de nunca bloquear a
    Audio Thread (`standards/realtime_audio.md`).

- **Negativas:**
  - (a) **Sem auto-wiring** — cada dependência deve ser registrada manualmente;
    isso é repetitivo e um pouco verboso. Mitigação: o boilerplate fica confinado
    em `register_*`, não espalhado.
  - (b) **Registration implícito pode mascarar novos requisitos** — se uma
    engine ganhar uma nova dependência e o `register_*` não for atualizado, o
    erro só aparece em runtime (`std::runtime_error("unregistered: ...")`).
    Mitigação: exigir **testes de wiring** (do tipo "se `register_music_theory`
    foi chamado, `resolve<IChordEngine>()` não lance") na suíte de testes,
    como parte do DoD da Fase 1.1.
  - (c) **`std::type_index` tem custo de hash / string em debug** — irrelevante
    na hot path porque o `ServiceLocator` vive fora dela (setup time), e em
    release o custo é uma hash por `resolve`. Desprezível frente ao trabalho
    real das engines.
  - (d) **`shared_ptr` em todos os lugares** — overhead de refcount para 13
    engines é despreizável mas não zero. Aceitável porque o MTE não instancia
    engines por evento de áudio; elas vivem por todo o lifetime do processo.

- **Neutras:**
  - (a) Coincide com a abordagem já existente em `make_harmony_engine` (que já
    recebe `std::shared_ptr<IScaleProvider>`) — a mudança é ergonômica, não
    estrutural.
  - (b) Coincide com §7 do `prompts/master_prompt.md` ("Sempre utilizar DI")
    sem especificar framework — esta ADR fixa a escolha.
  - (c) O boundary com Go (ADR-0001) não é afetado — o `ServiceLocator` vive
    inteiramente no lado C++/MTE e não cruza o socket gRPC.

## Relacionado

- ADR-0001 — Boundary C++ ↔ Go (gRPC socket vs FFI inline). Esta DI é no lado
  C++/MTE; o boundary gRPC não é impactado.
- ADR-0003 — Serialização do Shared Musical Context (proto vs JSON). O
  `RequestContext` que carrega seed/arena vive ao lado do `ServiceLocator`
  mas não é resolvido por ele.
- Skills: `cpp-templates`, `cpp-memory-management`, `modern-cpp`,
  `googletest` (para mocks via `MOCK_METHOD`).
- Standards: `standards/architecture.md` (§1 e §3), `standards/cpp_guidelines.md`,
  `standards/realtime_audio.md` (lifetime separado de estado de request).
- Roadmap: ver `docs/roadmap/ROADMAP.md` Fase 1.1 (Sprint 2 — "Container de DI
  C++ (boost::di ou ServiceLocator próprio) + ADR-0002").
- Código de referência: `engine/cpp/include/aimidi/theory/IHarmonyEngine.hpp`
  (fábrica atual manual com DI de `IScaleProvider`), `engine/cpp/include/aimidi/theory/IScaleProvider.hpp`
  (primeira interface com factory).
