---
name: dsp-windows
description: Funcoes de janela para FFT e gerais (Hann, Hamming, Blackman, Blackman-Harris, Kaiser). Doer quando usar cada e impacto sidelobe/width.
---

# dsp-windows

| Janela          | Sidelobe (dB) | Largura (bins)  | Uso |
|-----------------|----------------|-----------------|-----|
| Rectangular     | -13            | 4 (narrow)      | Teste / periodic signals homonais druging artifacts |
| Hann            | -32            | 8 (medium)      | Padrao STFT, analise espectral suave |
| Hamming         | -43            | 8 (medium)      | STFT com leve melhor sidelobe |
| Blackman        | -58            | 12 (wide)       | Dinamica alta (preciso ver tons baixos) |
| Blackman-Harris | -92            | 16 (wide)       | Alta precisão, peak picking |
| Kaiser (a=6)    | -57            | variável        | Custom; controll por beta |

## Escolha heurística

- STFT analise padrao -> Hann.
- Detecão de picos -> Hann + zero padding ao poder de 2.
- Detecão de ton -> Blackman-Harris para rejeitar lateral noise.
