---
name: dsp-filters
description: filtros biquad IIR (low/high/band pass, shelv EQ), one-pole para smoothing; coeficientes via RBJ cookbook; processamento vetorisado.
---

# dsp-filters

## Biquad (RBJ cookbook)

- Formato TransposedDirectForm II:
  ```
  y[n] = b0*x[n] + s1;  s1 = b1*x[n] - a1*y[n] + s2;  s2 = b2*x[n] - a2*y[n]
  ```
- Coeficientes `b0,b1,b2,a1,a2` por formula (lowpass/highpass/bandpass/notch/
  peakAllpass).

## one-pole smoothing

- `y[n] = a*x[n] + (1-a)*y[n-1]` para suave de parametros; `a = 1 - exp(-1/(tau*sr))`.

## Regras de MTE/DSP

- Implementar com `float` (SIMD friendly); double se houver demanda.
- Smoothing para cambios de parametro sem jumps.
- Nao usar `juce::dsp::IIR` em Audio Thread sem testar tempo deterministicamente.
