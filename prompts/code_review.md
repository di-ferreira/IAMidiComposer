# Prompt: Code Review

> Use para apresentar uma code review estruturada em PR given diff.

## Papéis

Peçao para um dos agentes agirem:

- @software-architect - impacto arquitetural, interfaces/DI
- @performance-engineer - hot paths, locks, allocs
- @security-engineer - parsers, IO, deps
- @qa-engineer - testes, golden, fuzz
- @music-theory-engineer - semância musical correctness
- @documentation-engineer - docs, ADRs

## Estrutura da review

### 1. Resumo (1-3 linhas)
PR sobre o que / qual impacto bootstrap.

### 2. Pontos positivos
Decisões boas, pattern aplicado, teste incluido.

### 3. Críticos / blocking
Listagem com `block:` e referencia a linha.

### 4. Melhorias sugeridas (nao blocking)
Listagem com `nit:` ou `sugest:`.

### 5. Verificação checklist
Conforme `standards/pull_request.md`.

## Output format example

```
## 1. Resumo
PR adiciona engine de harmonia II-V-I reaprovitando ChordEngine.

## 2. Positivos
- DI aplicada; factory preenchida.
- Golden MIDI incluído.
- Determinismo preserved (seed param).

## 3. Blocking
- block: `engine/cpp/src/theory/harmony/harmony.cpp:142` — std::mt19937 sem seed
  explicita no ctor; usar seeder da Blueprint.

## 4. Sugestoes (nit)
- nit: `progressao_t` typedefd em harmony.hpp; podia nome PascalCase.

## 5. Checklist
- [x] testes adicionar
- [x] golden atualizar
- [ ] ADR sobre mudança de fabrica (mencione)
- [x] docs/architecture/modules/theory-harmony.md atualizada
```
