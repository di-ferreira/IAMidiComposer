# `.opencode/` — Workspace do AI MIDI Composer

Este diretório contem:
- `agents/` - 38 agentes: 14 primários (engenheiros/liderança) + 24 subagentes
  (engines/musicais). Cada `.md` segue o formato frontmatter YAML do opencode
  (description, mode, model, permission, prompt).
- `skills/<name>/SKILL.md` - 56 skills granulares (C++20, Go, JUCE, VST3/CLAP,
  DSP, MIDI, Teoria Musical, IA, testing, security, doc). Descobertas
  automaticamente pelo opencode.
- `registry/` - Metadados YAML descritivos (extensão ao ROADMAP_WORKSPACE.md §Agent
  Registry). Complementar, não conflita com fontes:
  - `agents.yaml` - 14 primários
  - `subagents.yaml` - 24 subagentes
  - `skills.yaml` - 56 skills
  - `workflows.yaml` - 10 workflows musicais com sequências
  - `standards.yaml` - 13 standards

## Fonte canônica

A **fonte canônica** para prompts e permissões de cada agente é o arquivo
`.opencode/agents/<id>.md`. Os arquivos YAML em `registry/` apenas catalogam
metadados para descoberta externa (ferramentas de IAGE e automações).

## Naming

- Agentes e skills seguem o formato `^[a-z0-9]+(-[a-z0-9]+)*$` (validado pelo
  opencode). Todos os nomes em ingles ou kebab-case para deduplicar.
- Skills: kebab-case (uma pasta por skill).

## Como o opencode descobre

Ao iniciar na raiz do projeto, o opencode:
1. Carrega todos os `.md` em `.opencode/agents/` (primários viram agentes
   selecionaveis via Tab; subagentes via `@menção` ou por Task).
2. Carrega cada `.opencode/skills/<name>/SKILL.md` - Agents podem usar a tool
   `skill({ name: "..." })` sob demanda.
3. Carrega `opencode.jsonc` na raiz do projeto para permissões default.

## Restrições lembradas

- **Agentes não são fontes de codigo-fonte**: sao ferramentas para auxiliar a
  construcao. O codigo real nas pastas `plugin/`, `engine/cpp/`, `engine/go/`,
  `proto/`.
- Skills sao conhecimento on-demand: prefixe acesso com `skill({ name: "..." })`
  via tool de agent.
- O plano original (ROADMAP_WORKSPACE.md §"minha unica alteracao") previa
  `.workspace/registry`. Adaptamos para `.opencode/registry/` mantendo o conceito.
