# Testing — AI MIDI Composer

> Sem teste = sem merge. Sempre.

## Estratégia

- **Unit**: por modulo (engines, services, parser).
- **Integration**: workflow end-to-end (Plugin gRPC -> ACE -> MTE -> MIDI bytes).
- **Golden**: saida MIDI deterministica dada seed+blueprint (ver `golden-tests`).
- **Fuzz**: parsers MIDI/projeto/proto (ver `fuzz-testing`).
- **Performance**: benchmarks CI gate (ver `performance.md`).

## Piramide

```
        E2E (raro - 5%)        <- poucos, lentos (mock de DAW)
       /                \
   Integration (20%)              <- workflow completo sem DAW
   /                    \
 Unit (70%)                      <- cobre regras
 Fuzz (continuous)                <- em CI job dedicado
```

## Cobertura mínima

- **Music Theory Engine (C++)**: 85% linhas; golden para cada estilo/escala básica.
- **ACE Go (services)**: 80% linhas; 70% branches.
- **Musical Intelligence**: golden JSON para prompts-tipicos + tolerância model.
- **Plugin**: não exiger coverage em Audio Thread code (dificil); em UI logic sim.
- ***Decoder/parser/path*: 100% branches fncional; fuzz cobre corners.

## Teste musical

Cada PR de feature musical deve incluir:
- Golden MIDI (gerado e prod-save na PR).
- Um reviewers humano deve escutar o golden (path curto).
- Aceito: workflow explicitado no PR, instruções reprodução.

## CI gates

- CMake build + tests: `ctest --output-on-failure`.
- Go build + tests: `go test ./... -race -shuffle=on`.
- Lint: clang-format check, gofmt, golangci-lint, clang-tidy.
- Coverage report coletado; gate de regressão apenas na release branch.
- Fuzz (CIqualificado): 1h de runtime no master.

## Anti-padrao

- Testes flaky (aceitacao zero: se flakar, isolar e corrigir).
- Skip testes em "faz parte", "já testado", "pequeno": nao.
- Testes do musica descritos: "evita verificar resultado aleatorio"; use seed.
