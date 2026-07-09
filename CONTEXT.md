# AI MIDI Composer

## Visão Geral

O AI MIDI Composer é uma plataforma Open Source para composição musical assistida por Inteligência Artificial que funciona 100% offline.

O projeto NÃO é uma DAW.

O projeto NÃO produz áudio.

O projeto NÃO substitui instrumentos virtuais.

Seu objetivo é gerar MIDI inteligente, musicalmente coerente e totalmente editável, auxiliando produtores, compositores e músicos durante o processo criativo.

O usuário continuará utilizando sua DAW favorita (Cubase, Reaper, Studio One, FL Studio, Ableton Live, Bitwig, etc.) e seus VSTis preferidos.

O AI MIDI Composer atuará como um copiloto de composição.

---

## Missão

Democratizar a composição musical utilizando Inteligência Artificial local, preservando o controle criativo do músico.

A IA deve acelerar o processo criativo, nunca substituir o compositor.

O usuário sempre terá controle total sobre todas as decisões musicais.

---

## Filosofia

A Inteligência Artificial nunca deve gerar notas MIDI diretamente.

A IA apenas interpreta intenção musical.

A composição será realizada por uma Music Theory Engine determinística.

Toda música gerada deve poder ser reproduzida exatamente utilizando a mesma seed.

A teoria musical possui prioridade sobre respostas probabilísticas da IA.

Sempre priorizar previsibilidade, consistência e qualidade musical.

---

## Objetivos

Construir uma plataforma capaz de:

• interpretar prompts em linguagem natural

• compreender estilos musicais

• compreender emoção

• compreender instrumentação

• compreender estrutura musical

• sugerir arranjos

• gerar MIDI profissional

• analisar áudio existente

• complementar composições

• reorganizar arranjos

• gerar múltiplas variações

• reutilizar contexto musical

• funcionar completamente offline

---

## Escopo do Projeto

O projeto será dividido em dois grandes componentes.

### 1.

Plugin VST3

Responsável por:

Interface

Fluxo do usuário

Preview

Piano Roll

Configuração

Integração com DAWs

Nunca realizará processamento pesado.

Nunca executará IA.

Nunca realizará composição.

---

### 2.

AI Composition Engine (ACE)

Executará:

Musical Intelligence

Planning Layer

Music Theory Engine

Audio Analysis

MIDI Rendering

Shared Musical Context

Instrument Mapping

Toda inteligência do projeto estará localizada aqui.

---

## Arquitetura Conceitual

Usuário

↓

Plugin

↓

AI Composition Engine

↓

Musical Intelligence

↓

Music Blueprint

↓

Planning Layer

↓

Music Theory Engine

↓

MIDI Rendering Engine

↓

Plugin

↓

DAW

---

## Papel da IA

A IA nunca gera MIDI.

A IA nunca gera áudio.

A IA nunca escreve notas.

A IA decide:

estilo

energia

instrumentação

estrutura

emoção

densidade

complexidade

groove

arranjo

Toda geração musical será responsabilidade da Music Theory Engine.

---

## Music Theory Engine

A Music Theory Engine é o coração do projeto.

Ela será totalmente determinística.

Ela será responsável por:

harmonia

escalas

modos

progressões

melodia

baixo

bateria

guitarra

piano

cordas

contraponto

voice leading

humanização

orquestração

Toda saída será MIDI.

---

## Shared Musical Context

Todo projeto possuirá um contexto compartilhado.

Ele armazenará:

tempo

tom

escala

estrutura

progressão

instrumentos

histórico

preferências

regiões bloqueadas

Todos os módulos trabalharão utilizando esse contexto.

---

## Workflows

O sistema será construído para suportar os seguintes fluxos de trabalho.

Workflow 1

New Composition

Criar uma composição completa a partir de um prompt.

Workflow 2

Instrument Composer

Gerar apenas um instrumento.

Workflow 3

Audio Assisted Composer

Analisar áudio existente e criar novos MIDIs.

Workflow 4

Continue Composition

Continuar uma composição existente.

Workflow 5

Smart Regeneration

Regenerar apenas uma região.

Workflow 6

Generate Variations

Criar variações mantendo identidade.

Workflow 7

Replace Instrument

Adaptar MIDI para outro instrumento.

Workflow 8

Reharmonize

Criar nova harmonia preservando melodia.

Workflow 9

Orchestrate

Transformar uma ideia simples em arranjo orquestral.

Workflow 10

Arrange

Criar acompanhamento automaticamente.

---

## Princípios Técnicos

O projeto deve ser:

100% Offline

Cross Platform

Open Source

Modular

Escalável

Determinístico

Preparado para IA local

Baixa latência

Alta performance

Baixo consumo de memória

Alta testabilidade

Baixo acoplamento

Alta coesão

---

## Stack Tecnológica

Plugin

C++20

JUCE

Engine

C++20

Go

Comunicação

gRPC

Protocol Buffers

Banco

SQLite

IA

llama.cpp

GGUF

ONNX Runtime

Build

CMake

Testes

GoogleTest

Logs

spdlog

CI/CD

GitHub Actions

---

## Restrições

Nunca bloquear Audio Thread.

Nunca utilizar mutex na Audio Thread.

Nunca realizar IO durante processamento de áudio.

Nunca executar IA na Audio Thread.

Nunca utilizar alocação dinâmica em tempo real.

Nunca confiar em dados vindos da interface.

Nunca criar dependências circulares.

Nunca misturar UI com regras de negócio.

Nunca misturar IA com teoria musical.

Nunca gerar MIDI diretamente pela IA.

---

## Objetivo Final

Ao término do projeto, o AI MIDI Composer deverá ser uma plataforma completa de composição musical assistida por IA, composta por:

• Plugin VST3 profissional.

• AI Composition Engine independente.

• Music Theory Engine modular.

• Audio Analysis Engine.

• Shared Musical Context.

• Sistema de Instrument Mapping.

• Biblioteca de estilos musicais.

• Sistema de Workflows.

• Sistema de Seeds reproduzíveis.

• Biblioteca de padrões musicais.

• Sistema de humanização.

• API para novos módulos.

• Arquitetura para criação de novos instrumentos.

• Sistema de templates.

• Sistema de presets.

• Estrutura preparada para colaboração Open Source.

O projeto deverá permitir que qualquer desenvolvedor implemente novos módulos musicais sem alterar a arquitetura principal.

Toda nova funcionalidade deverá respeitar a arquitetura oficial.

A arquitetura sempre terá prioridade sobre velocidade de implementação.

O código deverá permanecer modular, altamente performático e preparado para evolução pelos próximos anos.

---

## Definição de Sucesso

O projeto será considerado concluído quando:

• Todos os workflows estiverem implementados.

• Todos os módulos forem desacoplados.

• O sistema funcionar totalmente offline.

• O plugin for compatível com as principais DAWs.

• Toda geração musical ocorrer pela Music Theory Engine.

• Toda IA for local.

• Toda documentação estiver atualizada.

• O projeto possuir testes automatizados.

• O projeto possuir benchmarks públicos.

• O projeto puder ser utilizado como referência Open Source para desenvolvimento de software musical baseado em IA.
