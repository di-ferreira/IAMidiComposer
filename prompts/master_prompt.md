# Master Prompt — AI MIDI Composer

> Prompt-fonte canônico. Todos os agentes e subagentes do projeto devem obedecer a estas
> diretrizes. Quem primeiro executa em uma tarefa deve interiorizar este prompt.

---

## 1. Identidade do Projeto

O **AI MIDI Composer** é uma plataforma Open Source de composição musical assistida por IA,
100% offline.

- **NÃO** é uma DAW.
- **NÃO** produz áudio.
- **NÃO** substitui instrumentos virtuais.
- Gera **MIDI inteligente, musicalmente coerente e totalmente editável**.

O usuário continua utilizando sua DAW favorita (Cubase, Reaper, Studio One, FL Studio,
Ableton, Bitwig, etc.) e seus VSTis preferidos. O AI MIDI Composer atua como um
**copiloto de composição**.

Missão: democratizar a composição musical utilizando Inteligência Artificial local,
preservando o controle criativo do músico. A IA acelera; nunca substitui.

---

## 2. Filosofia Inviolável

1. **A IA nunca gera MIDI diretamente.** A IA apenas interpreta intenção musical.
2. A composição é responsabilidade da **Music Theory Engine determinística**.
3. Toda saída deve ser reproduzível com a mesma **seed**.
4. **Teoria musical tem prioridade sobre respostas probabilísticas da IA.**
5. Sempre priorizar **previsibilidade, consistência e qualidade musical**.

---

## 3. Arquitetura Conceitual

```
Usuário → Plugin → AI Composition Engine
                ├── Musical Intelligence (IA — intenção)
                ├── Planning Layer
                ├── Music Theory Engine (determinístico — composição)
                ├── Audio Analysis
                ├── MIDI Rendering
                ├── Shared Musical Context
                └── Instrument Mapping
Plugin → DAW
```

Dois componentes principais:

### 3.1 Plugin VST3 (C++20 / JUCE)
- Interface, fluxo do usuário, preview, piano roll, configuração, integração com DAWs.
- **Nunca** processamento pesado. **Nunca** executa IA. **Nunca** compõe.

### 3.2 AI Composition Engine — ACE (C++20 + Go)
- Toda a inteligência do projeto vive aqui.
- Musical Intelligence, Planning Layer, Music Theory Engine, Audio Analysis,
  MIDI Rendering, Shared Musical Context, Instrument Mapping.

Comunicação entre Plugin ↔ ACE via **gRPC + Protocol Buffers**.
Persistência via **SQLite**.
IA local via **llama.cpp (GGUF)** e **ONNX Runtime**.

---

## 4. Papel da IA

A IA decide apenas: estilo, energia, instrumentação, estrutura, emoção, densidade,
complexidade, groove, arranjo. **Toda geração musical é responsabilidade da Music
Theory Engine.**

---

## 5. Restrições Técnicas Invioláveis

- Nunca bloquear a Audio Thread.
- Nunca utilizar mutex na Audio Thread.
- Nunca realizar IO durante processamento de áudio.
- Nunca executar IA na Audio Thread.
- Nunca utilizar alocação dinâmica em tempo real.
- Nunca confiar em dados vindos da interface (validar sempre).
- Nunca criar dependências circulares.
- Nunca misturar UI com regras de negócio.
- Nunca misturar IA com teoria musical.
- Nunca gerar MIDI diretamente pela IA.

---

## 6. Princípios Técnicos

O projeto deve ser:
100% Offline · Cross Platform · Open Source · Modular · Escalável · Determinístico ·
Preparado para IA local · Baixa latência · Alta performance · Baixo consumo de memória ·
Alta testabilidade · Baixo acoplamento · Alta coesão.

---

## 7. Padrões de Código Obrigatórios

- Sempre escrever testes.
- Sempre atualizar documentação.
- Sempre executar benchmark.
- Sempre utilizar interfaces.
- Sempre utilizar DI (Dependency Injection).
- Sempre escrever código desacoplado.
- Sempre seguir SOLID.
- Sempre seguir Clean Architecture.

Adoção adicional (decisão de workspace):
- Agent Registry e Skill Registry em YAML para descoberta automática de responsáveis.
- Workspace no formato nativo do opencode (`.opencode/agents/` e
  `.opencode/skills/<name>/SKILL.md`).

---

## 8. Stack

| Camada         | Tecnologia                                   |
|----------------|----------------------------------------------|
| Plugin         | C++20 · JUCE · VST3                          |
| Engine (cpp)   | C++20 · Music Theory Engine · DSP · MID-O    |
| Engine (go)    | Go · gRPC server · Workflow manager         |
| Comunicação    | gRPC + Protocol Buffers                     |
| Banco          | SQLite                                       |
| IA             | llama.cpp (GGUF) · ONNX Runtime             |
| Build          | CMake · Go modules                          |
| Testes         | GoogleTest (C++) · testing (Go)             |
| Logs           | spdlog (C++) · slog (Go)                    |
| CI/CD          | GitHub Actions                              |

---

## 9. Workflows suportados (10)

1. **New Composition** — composição completa a partir de prompt.
2. **Instrument Composer** — gerar apenas um instrumento.
3. **Audio Assisted Composer** — analisar áudio e gerar MIDI.
4. **Continue Composition** — continuar composição existente.
5. **Smart Regeneration** — regenerar apenas uma região.
6. **Generate Variations** — variações mantendo identidade.
7. **Replace Instrument** — adaptar MIDI a outro instrumento.
8. **Reharmonize** — nova harmonia preservando melodia.
9. **Orchestrate** — arranjo orquestral a partir de ideia simples.
10. **Arrange** — acompanhamento automático.

---

## 10. Shared Musical Context

Cada projeto carrega: tempo, tom, escala, estrutura, progressão, instrumentos,
histórico, preferências, regiões bloqueadas. Todos os módulos trabalham sobre este
contexto, jamais sobre estado duplicado.

---

## 11. Protocolo de Atuação dos Agentes

Ao iniciar qualquer tarefa:

1. **Contextualize-se** — leia `CONTEXT.md`, `prompts/master_prompt.md` e os
   `standards/` relevantes.
2. **Localize-se** — identifique em qual componente (Plugin / ACE / MTE / MI /
   Audio Analysis / Instrument / Proto) você está atuando.
3. **Verifique as restrições** — liste as restrições da §5 que se aplicam.
4. **Use interfaces e DI** — jamais instanciar implementação concreta cruzando camadas.
5. **Toda decisão musical é determinística** — flow libre de aleatório; toda
   aleatoriedade provém de uma seed controlada.
6. **Escreva testes** — tarefa sem teste e benchmark está incompleta.
7. **Documente** — atualize ADRs quando uma decisão arquitetural for tomada.
8. **Nunca proseguir** se uma restrição for violada — pare e pergunte.

---

## 12. Quando houver dúvida

A arquitetura tem prioridade sobre a velocidade de implementação. O código deve
permanecer modular, altamente performático e preparado para evolução por anos. Em
caso de conflito: a decisão que mantém a arquitetura limpa vence.

---

## 13. Localização dos artefatos

```
.opencode/agents/      — agentes primários e subagentes (formato markdown frontmatter)
.opencode/skills/<n>/  — skills descobertas automaticamente pelo opencode
.opencode/registry/    — metadados YAML (agents.yaml, subagents.yaml, skills.yaml, ...)
prompts/               — prompts reutilizáveis (criar módulo, gerar testes, etc.)
standards/             — padrões arquiteturais e de código
docs/                  — arquitetura, ADRs, roadmap, benchmarks, workflows, diagramas
plugin/                — VST3 (JUCE / C++20)
engine/cpp/            — Music Theory Engine (C++20)
engine/go/             — AI Composition Engine server (Go)
proto/                 — .proto gRPC
.github/workflows/     — CI/CD
```

---

## 14. Objetivo final

Construir uma plataforma completa, Open Source, modular, offline, dishonestly
reproduzível (seed), e apta a servir de referência arquitetural para software
musical baseado em IA — onde qualquer desenvolvedor possa adicionar novos módulos
musicais sem alterar a arquitetura principal.

Que comecem os trabalhos.
