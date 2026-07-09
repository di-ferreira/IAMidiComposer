# Standards - Padrões do Projeto

Esta pasta define os padrões **obrigatórios** que todo o projeto AI MIDI Composer
segue. Agentes e Skills referenciam estes documentos.

## Documentos

| Arquivo          | Escopo                                                    |
|------------------|-----------------------------------------------------------|
| `architecture.md`  | Princípios arquiteturais globais (Clean Arch + DI + SOLID) |
| `coding_style.md`  | Estilo de código (naming, files, headers, formatação)      |
| `cpp_guidelines.md`| Regras específicas de C++20                               |
| `go_guidelines.md` | Regras específicas de Go                                   |
| `realtime_audio.md`| Regras invioláveis para a Audio Thread                     |
| `performance.md`   | Critérios de aceitação de performance em PRs              |
| `security.md`      | Restrições de segurança local/offline                     |
| `testing.md`       | Estratégia de testes e cobertura mínima                    |
| `branching.md`     | Estratégia de branches (`trunk-based` / `release/` etc.)   |
| `commits.md`       | Estilo de Commits (Conventional Commits)                  |
| `pull_request.md`  | Estrutura de PR + checklist                                |
| `review.md`        | Critérios de code review                                   |
| `documentation.md` | Padrões de documentação                                    |

## Invioláveis

Estes padrões são **obrigatórios**. PRs que os violem são rejeitados pela QA.
O `prompts/master_prompt.md` tem precedência sobre qualquer ambiguidade aqui.
