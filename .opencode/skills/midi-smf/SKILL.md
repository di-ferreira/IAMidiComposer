---
name: midi-smf
description: Standard MIDI File (SMF) - formatos 0/1/2, header chunk (MThd), track chunks (MTrk), variable length quantity (VLQ), regra de delta-time.
---

# midi-smf

## Header (MThd)

```
4D 54 68 64       "MThd"
00 00 00 06       length = 6
00 <format><format><ntrks><ntrks><division><division>
```

- **format**: 0 (single track), 1 (multi-track synchronized), 2 (multi-track
asynchronous - nao usado).
- **division**: PPQ (positivo) ou SMPTE (negativo).
- Padrao do projeto: gerar **SMF1** com 1 trilha por instrumento; export "SMF0"
  como opcao simplificada.

## Tracks (MTrk)

- Cada MTrk: trilha contendo eventos precedidos de **delta-time** (VLQ).
- VLQ: 7 bits por byte com bit de continuacao.

## Padrao no projeto

- Tiny escritor em C++ sem deps externas; usando ao tamanho pre-alocavel.
- Testes golden: `engine/cpp/tests/midi/smf_writer_test.cpp` + arquivos `.mid` 
  esperados para regresssao.
