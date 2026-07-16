# Benchmarks — AI MIDI Composer v0.1.0

> Last updated: 2026-07-16
> Hardware: Intel Core i7-12700H, 32GB DDR5, Linux

## MTE Pipeline Benchmarks

| Benchmark | 4 bars | 16 bars | 64 bars | Target |
|---|---|---|---|---|
| HarmonyEngine | 0.00183 ms | 0.00354 ms | N/A | <1ms |
| ChordEngine | N/A | N/A | N/A | <1ms |
| BassEngine | N/A | N/A | N/A | <1ms |
| DrumEngine | N/A | N/A | N/A | <1ms |
| PianoEngine | N/A | N/A | N/A | <1ms |
| Full Pipeline | 0.37 ms | 1.35 ms | N/A | <10ms |
| SMF Render (1000 events) | N/A | N/A | N/A | <1ms |

## DSP Benchmarks

| Benchmark | 1s audio | 10s audio | 60s audio | Target |
|---|---|---|---|---|
| FFT (1024) | N/A | N/A | N/A | <1ms |
| Onset Detection | N/A | N/A | N/A | <10ms |
| Key Detection | N/A | N/A | N/A | <10ms |
| Chord Detection | N/A | N/A | N/A | <10ms |

## MI Pipeline Benchmarks

| Benchmark | Time | Target |
|---|---|---|
| Prompt Interpretation | N/A | <100ms |
| Blueprint Generation | N/A | <50ms |
| Timeline Planning | N/A | <10ms |
| Arrangement Planning | N/A | <10ms |
| Full MI Pipeline | N/A | <500ms |

## End-to-End Benchmarks

| Workflow | 4 bars | 16 bars | Target |
|---|---|---|---|
| W1 New Composition | N/A | N/A | <5s |
| W2 Instrument Composer | N/A | N/A | <2s |
| W3 Audio Assisted | N/A | N/A | <10s |
| W8 Reharmonize | N/A | N/A | <3s |
| W9 Orchestrate | N/A | N/A | <5s |
