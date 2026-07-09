---
name: input-validation
description: Validacao de entrada em plugin e engine - tamanho max de MIDI/projeto, limites de madigade track/file, sanitize prompt de usuario antes de enviar ao LLM.
---

# input-validation

## Regras

- Plugin -> ACE: validar todos campos do proto request (proto-gen-validate).
- Import de .mid/.json: limites (10 MB) + parser robusto (sem crash em mal-formed).
- Prompt do usuario: tamanho max (ex.: 2k chars), strings sanitizadas (sem bytes nulos
  injetados).
- Audio import: limites de duration / sampate / channels razoáveis.
- Sem execao de code path; apenas erros structurados (`Status` gRPC).

## Anti-padroes

- Acreditar em tamanhos do host sem validar.
- Skip validacao em debug builds.
- Logs com payload completo do usuario (privacy).
