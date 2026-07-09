---
name: model-safety
description: Seguranca de modelos locais (GGUF/ONNX) - checksums, sandbox de fetch, capacidades minimas, inspeção de metadados, sem auto-download de fontes nao confidaveis.
---

# model-safety

## Regras

- Download de modelos via script versionado com checksum SHA256 + tamanho
  declarado.
- Fonte: HuggingFace com namespace organizacional confiavel (ex.: `TheBloke`,
  oficiais).
- Inspecionar metadados GGUF ONNX key `general.architecture`, `tokenizer.ggml.*`;
  rejeitar hashes desconhecidos.
- Modelos armazenados em `engine/cpp/data/models/` (gitignored); nunca commitar.
- Não executar modelos baixados automaticamente em build (CI); uso manual +
  assinatura.

## Anti-padroes

- Carregar modelos a partir de caminho relativo sem checksum (supply chain).
- Trace de prompts em logs; se necessario, mascarar.
- Inference sem timeout/abort (DoS interno).
