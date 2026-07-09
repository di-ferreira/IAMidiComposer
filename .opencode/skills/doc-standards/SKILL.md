---
name: doc-standards
description: Padroes de documentacao - Markdown leve, headings H1 por arquivo apenas, diagramas em PlantUML em docs/diagrams/, code examples curtos focados, evite duplicar informacao.
---

# doc-standards

## Regras de estilo

- Um H1 por arquivo; depois H2 para seções principais, H3 em diante.
- 80-120 colunas por linha (markers visuais); paragrafos curtos.
- Code blocks com linguagem flag (` ```cpp `) para syntax.
- Diagramas: PlantUML em `docs/diagrams/` com `.puml`; render por CI.
- Tabelas para dados estruturados; listas para passos.

## Anti-padraos

- Repetir a mesma informacao em multiplas docs; prefira fonte unica + link.
- Emojis em documentos tecnicos (mantenha gravidade).
- Screenshots no lugar de texto (perigo staleness).
- Markdown pre-rebuild HTML commitado (gerado via CI se necessario).

## Manutençao

- Cada doc tem um "Owner" (Engenheiro responsavel).
- Staleness gate: docs revisados a cada 6m; ADRs são permanentes.
