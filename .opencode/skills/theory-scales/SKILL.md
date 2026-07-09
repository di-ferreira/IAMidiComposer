---
name: theory-scales
description: Escalas musicais: maior, menor natural/harmonica/melodica, pentatonicas maior/menor, blues, exotica (modo hirajoshi, hungaria). Mapa intervalar em semitons.
---

# theory-scales

## Escalas comuns (intervalos em semitons a partir da tonica)

- **Maior (Ionian)**: 0 2 4 5 7 9 11
- **Menor natural (Aeolian)**: 0 2 3 5 7 8 10
- **Menor harmonica**: 0 2 3 5 7 8 11
- **Menor melódica (asc.)**: 0 2 3 5 7 9 11
- **Pentatônica maior**: 0 2 4 7 9
- **Pentatônica menor**: 0 3 5 7 10
- **Blues**: 0 3 5 6 7 10

## Remedios exóticos (uso modal)

- **Hirajoshi** (japonês): 0 2 3 7 8
- **Húngaro menor**: 0 2 3 6 7 8 11
- **Phrygian dominant**: 0 1 4 5 7 8 10

## Padrao de uso na engine

- Definir em `constexpr` arrays de semitons (`Scales`).
- Para gerar seqüência: usar transposicao desda root key.
- Maioria das escalas é heptatônica; pentatonicas e exóticas usos spécifics.
- Sempre retornar um `std::span<const int>` em O(1).
