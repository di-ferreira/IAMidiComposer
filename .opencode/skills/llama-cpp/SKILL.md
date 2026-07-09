---
name: llama-cpp
description: Integração com llama.cpp (GGUF) - carregamento de modelo, geracao com seed, temperature low, prompt format, threads vinculadas a cores isoladamente da Audio Thread.
---

# llama-cpp

## Padrao no projeto

- Biblioteca embutida via CMake `FetchContent` ou submodule; sem dependencias de
  nuvem na build.
- Modelos GGUF em `engine/cpp/data/models/` (gitignored), baixados por script
  `scripts/fetch_models.sh` com checksum SHA256.
- **Threads de inferencia**: configurar via CPU cores -1 (preservar audio thread
  ficar em seu core).
- **Sampling seed** explicito em cada chamada; cache por hash(prompt+contexto).

## Configuracoes frequentes

- `n_ctx`: 4096 (contexto suficiente para prompts de composicao completa).
- `temperature`: 0.2 a 0.4 (foco em consistencia).
- `top_p`: 0.9, `top_k`: 40, `repeat_penalty`: 1.1.
- `seed`: derivada de seed da Blueprint + step workflow.

## Anti-padroes

- Carregar model em Audio Thread (proibido).
- Inferencia em Audio Thread (proibido).
- Texto livre como downstream (sempre parse IO JSON).
- Acessar `llama_get_timings` em hot path (caro -  apenas logs).
