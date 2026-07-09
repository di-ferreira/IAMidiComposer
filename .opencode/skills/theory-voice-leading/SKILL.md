---
name: theory-voice-leading
description: Voice leading suave entre acordes - minimizafao de movimento, tons comuns, evitar paralelos de quinta/oitava, configuracao close/open voicings.
---

# theory-voice-leading

## Principios

- **Tons comuns**: manter nota comum no mesmo voice entre dois acordes consecutivos.
- **Menor movimento**: mover cada voz o minimo (de preferência por grau conjunto).
- **Lead com a voz superior**: melodia guia escolha de inversao.
- **Sem paralelos de 5a/8a** entre voices (classical/jazz cadence rules).

## Tipos de voicing

- **Close**: vozes aglomeradas (ate uma oitava entre adjacent); para teclas.
- **Open**: vozes espacadas por oitavas; para cordas/corais.
- **Drop2/Drop3** (jazz): dropando 2a/3a voz do top em ordered chord -timbre
  mais aberto.
- **Spread triads**: alternar desar/fechado para funcoes claras.

## Algoritmo (MTE)

1. Recebe acorde1 e acorde2 listas de notas (simbolos).
2. Para cada voz de acorde1: enachar closest consonant pitch em acorde2
   respeitando regras.
3. Penalizar: paralelos de 5a/8a, saltos > 3 intervalos, crosses entre voices.
4. Retornar minimos (DP) ou heurística por prioridade de voz.
