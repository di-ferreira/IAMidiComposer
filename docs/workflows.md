# Workflows — AI MIDI Composer

> Guia de uso dos agentes via menção `@agente` no opencode, e mapa dos 10 workflows
> musicais suportados.

## Como invocar agentes no opencode

Pré-requisito: o opencode descobre automaticamente todos os agentes em
`.opencode/agents/*.md` e todas as skills em `.opencode/skills/<name>/SKILL.md`.

### Agentes primários (ciclar com Tab ou via `@nome`)

| Quando quero...                                  | Invoque        |
|--------------------------------------------------|----------------|
| Decidir trade-offs arquiteturais                 | `@cto`         |
| Desenhar/modular um subsistema                   | `@software-architect` |
| Planejar próxima sprint, quebrar epic            | `@project-manager` |
| Auditar/revisar hot path, exigir bench           | `@performance-engineer` |
| Atualizar doc / criar ADR                        | `@documentation-engineer` |
| Implementar gRPC/Go na ACE                       | `@backend-engineer` |
| Implementar VST3/JUCE/UI/piano roll              | `@plugin-engineer` |
| Implementar DSP/FFT/onset                        | `@dsp-engineer` |
| Implementar harmonia/melodia/etc (musica!)        | `@music-theory-engineer` |
| Implementar MI (llama/ONNX), interpretar prompt   | `@ai-engineer` |
| Schema SQLite, migrations                         | `@database-engineer` |
| Threat model / validacao / dependencias          | `@security-engineer` |
| Estratégia de testes, golden MIDI, fuzz          | `@qa-engineer` |
| CI/CD, build matrix, releases                    | `@devops-engineer` |

### Subagentes (`@nome`, geralmente invocados por agentes maiores)

Os subagentes normalmente são acionados por `@music-theory-engineer` ou
`@ai-engineer`, mas podem ser chamados diretamente:

- MTE: `@harmony-engine` `@chord-engine` `@melody-engine` `@rhythm-engine`
  `@bass-engine` `@drum-engine` `@guitar-engine` `@piano-engine`
  `@strings-engine` `@orchestration-engine` `@humanization-engine` `@midi-renderer`
- MI: `@prompt-interpreter` `@style-detection` `@blueprint-generator`
  `@arrangement-planner` `@timeline-planner` `@workflow-manager`
- Audio: `@tempo-detection` `@key-detection` `@beat-detection` `@chord-detection`
- Instrument: `@plugin-scanner` `@preset-manager` `@instrument-mapper`

## Workflows musicais (10)

Cada workflow tem sua sequência de subagentes; ver `.opencode/registry/workflows.yaml`
para mapeamento completo. Docs detalhados em `docs/workflows/<NN>-*.md` (a preencher
conforme implementação).

| #  | Workflow                  | Resumo                                                      |
|----|---------------------------|-------------------------------------------------------------|
| 1  | New Composition           | Composição completa a partir de prompt.                     |
| 2  | Instrument Composer        | Gerar apenas um instrumento.                                |
| 3  | Audio Assisted Composer    | Analisar áudio existente e gerar MIDI coerente.             |
| 4  | Continue Composition       | Continuar composição existente.                             |
| 5  | Smart Regeneration         | Regenerar apenas uma região.                                |
| 6  | Generate Variations        | Variações mantendo identidade.                              |
| 7  | Replace Instrument         | Adaptar MIDI para outro instrumento.                        |
| 8  | Reharmonize                | Nova harmonia preservando melodia.                          |
| 9  | Orchestrate                | Idea simples -> arranjo orquestral.                         |
| 10 | Arrange                    | Acompanhamento automático.                                   |

## Exemplos práticos

### Quero terminar a implementação da harmonia II-V-I

```
@music-theory-engineer Implementar progressão II-V-I na Harmony Engine de
acordo com a skill theory-harmony. Use ADR eventual. Adicionar golden MIDI com
seed 42 para C major.
```

### Quero pré-analisar um impacto arquitetural de trocar cgo por gRPC interno

```
@software-architect Avaliar a troca do binding cgo da MTE por gRPC interno
na ACE. Rascunhar ADR-0001 com prós/contras e alt. Sem alterar codigo.
```

### Quero que a QA me diga o que falta para um PR passar nos gates

```
@qa-engineer Verificar checklist do PR #123 (link), apontar pendencias
segundo standards/testing.md e standards/pull_request.md.
```

### Quero benchmarkar nova engine antes de merge

```
@performance-engineer Executar benchmark do harmony-engine (antes/depois)
no branch feat/mte-ii-v-i. Salvar em docs/benchmarks/mte-harmony.md.
```
