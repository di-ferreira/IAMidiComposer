# Benchmark Results — AI MIDI Composer v0.1.0

> Date: 2026-07-16  
> Hardware: Intel Core i7-12700H (14 cores / 20 threads), 32GB DDR5, Linux  
> Build: Release (-O3), GCC 14  
> Test executable: `aimidi_mte_tests` (GoogleTest)

## MTE Pipeline

| Test | Iterations | Mean time | Target | Status |
|---|---|---|---|---|
| HarmonyEngine 4 bars | 100 | **0.00183 ms** | <1ms | PASS |
| HarmonyEngine 16 bars | 50 | **0.00354 ms** | <5ms | PASS |
| Full Pipeline 4 bars | 20 | **0.3685 ms** | <10ms | PASS |
| Full Pipeline 16 bars | 10 | **1.3495 ms** | <50ms | PASS |

### Raw output

```
[BENCH] HarmonyEngine 4 bars: 0.00183 ms
[BENCH] HarmonyEngine 16 bars: 0.00354 ms
[BENCH] Full pipeline 4 bars: 0.3685 ms
[BENCH] Full pipeline 16 bars: 1.3495 ms
```

## Notes

- HarmonyEngine benchmarks target the core harmonic progression generation (II-V-I patterns) and delegate voicing to ChordEngine.
- Full Pipeline includes harmony, chords, bass, rhythm, drums, piano, humanization, and SMF rendering.
- All benchmarks are seed-deterministic (seed=42) and run in Release mode with GoogleTest.
- Values marked **N/A** in the summary tables correspond to components that do not yet have dedicated benchmark tests (planned for future sprints).
