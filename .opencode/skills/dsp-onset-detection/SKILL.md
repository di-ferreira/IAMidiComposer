---
name: dsp-onset-detection
description: Deteccao de onsets - fluxo complexo/energia espectral, HFC, complex domain. Base para detectacao de beats e de BPM.
---

# dsp-onset-detection

## Metodos comuns

- **Energy-based** (time domain): soma RMS frame-by-frame; pico = onset.
- **Spectral flux**: soma da variacao de magnitude frame-to-frame.
- **HFC (High Frequency Content)**: penaliza baixas frequencias; util para percussao.
- **Complex domain**: mudancas em fase+amplitude.
- **Superflux** (modified flux): robusto a pitch variations.

## Pipeline basico

1. STFT (1024 win, 512 hop, Hann).
2. Compute feature (eg. spectral flux).
3. Smoothing / mediana filter.
4. Peak com adaptive threshold (multiplier of media estatisc) -> onset times.

## Aplica no projeto

- **Beat detection**: ons. + grid alignment.
- **BPM**: envelope de onset + autocorrelation.
- **Chord detection**: cada onset = novo segment harmonico.
