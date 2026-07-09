---
name: theory-rhythm
description: Ritmica musical - metricas (4/4, 3/4, 6/8, 7/8, 5/4), subdivisões (binaria/ternaria), stress patterns, poliritmia basica, swing factor.
---

# theory-rhythm

## Metricas

- **Simples** (bin/quad): 2/4, 3/4, 4/4
- **Composta** (ternaria): 6/8, 9/8, 12/8
- **Impares**: 5/4, 5/8, 7/8, 7/4
- **Mistura**: alocada por compasso por `Timeline` (Blueprint).

## Subdivisões

- **Binaria**: 4 4 16 16 16 16 (4 sub de 16)
- **Ternaria**: 12 8 6/8 -> tres notas por cada col
- **Swing**: ex. 16th swing -> long-short-long-short; factor 50=arrowed 55-65 tipico

## Stress patterns

- Notas fortes nas downbeats (1, 3 em 4/4), notas medias (2, 4), fracas (offs).
- Hipermetric structure: 4-bar phrase (antecedente + consequente).

## Poliritmia basica

- **Hemiola** (3 contra 2): 3 notas em valor de 2 - sempre syncopated.
- **Cross-rhythm** (3 over 2/4): usar com cautela, define por genero.

## Padrao de uso na engine

- Definir `Beat` em 480 PPQ (padrão DAW) como unidade.
- Grid e gerado como `ticks`; intelligente dentro do tempo do compasso.
