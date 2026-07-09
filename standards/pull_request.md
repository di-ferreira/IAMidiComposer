# Pull Request — AI MIDI Composer

## Template

```md
## Resumo
<1-3 frases; o que e por que>

## Tipo de mudança
- [ ] feat
- [ ] fix
- [ ] perf
- [ ] refactor
- [ ] docs
- [ ] test
- [ ] build/ci

## Checklist
- [ ] Adere a `prompts/master_prompt.md` (restricoes violadas?).
- [ ] Passa `clang-format` / `gofmt` / `golangci-lint` / `clang-tidy`.
- [ ] Passa typecheck / build.
- [ ] Testes adicionados/atualizados.
- [ ] Benchmark comparativo incluido (se hot path).
- [ ] Golden MIDI atualizado (se feature musical).
- [ ] Documentação relevante atualizada (ADR se decisão arq).
- [ ] Sem secrets no diff.
- [ ] CHANGELOG atualizado.

## Mudanças arquiteturais (preencha se relevante)
- [ ] Adicionei/actualizei ADR em `docs/adr/`.
- [ ] Atualizei diagramas em `docs/diagrams/`.

## Como testar
<comandos para reproduzir -> `cmake --build . && ctest -R HarmonyGolden`>

## Riscos/Trade-offs
<se houver>

## Issue/ADR relacionados
- Closes #NNN, Relates ADR 0007
```

## Code Review

- Minimo 1 reviewer do mesmo dominio + 1 cross-domain quando arquitetura.
- **Performance Engineer** em `perf`/hot path changes.
- **Software Architect** em mudancas de proto/interface.
- **Security Engineer** em parser/IO/dep.
- **Documentation Engineer** em docs/ADR.
