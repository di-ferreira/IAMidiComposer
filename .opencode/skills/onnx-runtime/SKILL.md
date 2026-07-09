---
name: onnx-runtime
description: Uso do ONNX Runtime para classificadores auxiliares (style/mood/tag). Sessao single-site, com fornecedor de CPUimatriz.
---

# onnx-runtime

## Padrao

- Pacote `onnxruntime` via CMake FetchContent; modelo `.onnx` em
  `engine/cpp/data/models/` gitignored ( hạ checksum).
- Single shared session para style/mood (num-threads: max(2, N-2)).
- Input via Ort::Value; output tensor para parse.

## Cache

- Results cacheados por hash de features (BPM/key/timbre summary).
- TTL infinito modelo version constante.

## Anti-padroes

- Sessao em Audio Thread (proibido).
- Input sem shape validation (acident_size-skmaxe crash).
- Alocacao de tensores em cada inferencia; reuse Ort::MemoryInfo + arena.
