---
name: dsp-fft
description: FFT (radix-2, pffft perso) para analise espectral; janelas (Hann/Hamming/Blackman); overlap-add para STFT.
---

# dsp-fft

## Padroes

- **Bib**: `pffft` (single-precision, fast) ou `kissfft` para C++.
- **STFT**: window + FFT hop-by-hop; para analises de BPM/key/chord.
- **Janelas**: Hann (default), Hamming (mild sidelobes), Blackman (sharp stops).
- **Hop**: N//2 (default 50%); 75% para qualidade/overlap.

## Cuidados

- Reuse buffers; arena em analises multi-passo.
- Blocar em SIMD (`pffft` ja interno).
- Evitar alocar em loops (aloca fora do hot path).

## Validacao

- Input DC => peak em DC (bin 0).
- Test tone A=440 -> peak em bin proximo.
