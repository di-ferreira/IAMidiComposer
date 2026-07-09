# Commits — AI MIDI Composer

## Estilo: Conventional Commits

```
<type>(<scope>): <subject>

<body opcional>

<footer opcional (Breaking change/Fixes/Closes)>
```

### Types

- `feat`: nova feature
- `fix`: correção
- `docs`: documentação
- `refactor`: refatoração sem mudança de comportamento
- `perf`: melhoria de performance
- `test`: testes
- `build`: build/CI/dep
- `ci`: pipeline
- `chore`: misc nao-functional
- `revert`: revert

### Scope (sugestões)

- `mte`, `mi`, `audio`, `plugin`, `proto`, `store`, `workflow`, `docs`, `ci`.

## Exemplos

```
feat(mte): II-V-I progressão em harmonia major
fix(plugin): underflow no FIFO MIDI out quando seed trocada
perf(midi): SIMD no rendering do SMF
docs(adr): novo ADR 0007 sobre cgo vs gRPC interno
```

## Regras

- Subject: imperativa, lowercase, sem ponto final, <= 72 chars.
- Body: 72 coluna; linha em branco entre subject/body.
- **BREAKING CHANGE**: no footer com explicação.
- Usar squash no merge; PR title vira o commit final (mantenha conventional).
