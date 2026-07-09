# Prompt: Criar Módulo

> Use este prompt quando precisar criar um novo módulo musical/engine na Music
> Theory Engine ou em qualquer componente da ACE.

---

## Instruçães para o agente

Ao criar um novo módulo, siga rigorosamente:

### 1. Nome e escopo

- Identifique o módulo: `<domain>/<module-name>`.
- Modulo:
  - `theory` : musica deterministica (engine de harmonia, melody, etc.)
  - `mi`     : inteligencia artificial (interpreter, style, blueprint)
  - `audio`  : analise de audio (BPM/beat/key/chord)
  - `midi`   : parsing/serializacao MIDI
- Nome: snake_case.

### 2. Arquitetura

Dentro de `<domain>/`:

```
engine/cpp/include/aimidi/<domain>/<module>.hpp           # interface
engine/cpp/src/<domain>/<module>/<module>.cpp             # impl
engine/cpp/src/<domain>/<module>/<module>_test.cpp        # testes
engine/cpp/src/<domain>/<module>/CMakeLists.txt            # target lib + test
docs/architecture/modules/<domain>-<module>.md             # doc técnica
docs/adr/NNNN-add-<domain>-<module>.md                     # se decisao arquitetural
docs/benchmarks/<domain>-<module>.md                       # bench referencia
```

### 3. Interface

- Defina interface **pura** (`class I<Module>` ou namespace com functions de free
  function quando trivial).
- Use DI: instancia recebida via `Module(IConfig cfg, IDeps deps)`.
- Documente cada metodo com doc comment.

### 4. Determinismo

- Todo RNG derivado de `seed` recebida no ctor/`generate(... seed ...)`.
- Proibido: `std::random_device`, `time()`, global `rand` random global.

### 5. Teste

- Cobertura com golden quando aplicável (MIDI).
- Benchmark em `docs/benchmarks/<...>.md`.
- Fuzz se for parser/IO.

### 6. PR

- Branch: `feat/<domain>-<module>`.
- PR com checklist completo (ver `standards/pull_request.md`).
- Atualize `.opencode/registry/agents.yaml` se novo subagente Dev.

---

## Saída esperada

- Arquivos criados acima com impl + testes golden stub.
- Documento `docs/architecture/modules/<domain>-<module>.md` preenchido.
- Update em `docs/architecture/overview.md` adicionando modulo na lista.
