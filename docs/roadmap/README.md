# Roadmap — AI MIDI Composer

Plano canônico de desenvolvimento do projeto. Dono: **CTO Agent**.

## Index

- **[ROADMAP.md](ROADMAP.md)** — Plano completo em 8 fases (M0–M7), 30 sprints.
  - Fase 0 — Aterrissagem & Diagnóstico
  - Fase 1 — Foundation (gRPC, SQLite, DI, plugin bridge)
  - Fase 2 — Core Musical Pipeline / Spike "pop-rock 4/4" (primeiro som)
  - Fase 3 — Musical Intelligence local (LLM, prompt NL)
  - Fase 4 — MTE Horizontal Expansion (todos os instrumentos e modos)
  - Fase 5 — Audio Engine (Workflow 3)
  - Fase 6 — Plugin UI & DX (Piano Roll)
  - Fase 7 — Hardening, Benchmarks & Release v0.1.0

## Convenções

- Atualizações só via PR com aprovação de **CTO + Software Architect**.
- Cada nova fase ou mudança de milestone gera entrada na seção "Histórico de revisões" do `ROADMAP.md`.
- Toda decisão estrutural vira ADR em `../adr/` (ver `ADR-0001` e lista de previstos).
- Idioma: PT-BR (narrativo), termos técnicos em EN (ver `standards/documentation.md`).

## Relacionado

- [ADR Index](../adr/README.md)
- [Architecture Overview](../architecture/overview.md)
- [Workflows](../workflows/README.md)
- [Agent/Subagent Registry](../../.opencode/registry/)
- [Master Prompt](../../prompts/master_prompt.md)
- [Project Context (CONTEXT.md)](../../CONTEXT.md)
