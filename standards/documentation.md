# Documentation — AI MIDI Composer

> Documentacao e parte do produto. Sem ela, o projeto é privado mesmo sendo Open Source.

## Audiências

- **Usuarios** (produtores/musicistas/compositores): README, USER_GUIDE, GUI
  screenshots downstream.
- **Desenvolvedores Open Source**: docs/architecture/, standards/, ADRs.
- **Contribuidores novos**: CONTRIBUTING.md, onboarding quick-start.

## Estrutura

```
README.md                        - capa do projeto, quick start, badges
CONTEXT.md                       - missao, filosofia, escopo, objetivos
ROADMAP_WORKSPACE.md             - workspace design original (preservado)
CHANGELOG.md                     - Keep a Changelog (ver skill changelog)
CONTRIBUTING.md (futuro)         - como contribuir; setup de dev
SECURITY.md (futuro)             - policy de security reporting
docs/
    architecture/                - visao por modulos, diagramas textuais
    adr/                         - decisoes arquiteturais numeradas
    roadmap/                     - releases planejadas, delivered, beta
    benchmarks/                  - cada feature com bench
    workflows/                   - um documento para cada um dos 10 workflows
    diagrams/                    - PlantUML/C4 diagramas
standards/                       - regras do projeto (este + siblings)
prompts/                         - master_prompt.md e prompts reutilizáveis
```

## Audit staleness

- Review a cada 6 meses em `main`.
- ADRs sao permanentes; mudados via nova ADR.

## Idioma

- Projeto: PT-BR em CONTEXT/ROADMAP/contexts narrativos historically.
- **Código, comentários e ADRs: preferimos inglês** para internationalization.
  Quando padrao/regra definirl essa decisao via ADR, atualize padrao global.
- Doc narrativa de Open Source (README/CONTRIBUTING): Ingles cada vez que PR
  internacional (vetor: nascimento do project).
- Esta primeira release de workspace/wercits está em PT-BR por heranca do project
  original; PRs de internacionalização virão.
