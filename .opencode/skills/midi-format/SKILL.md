---
name: midi-format
description: Formato MIDI - eventos (note on/off, CC, PC, pitch bend), canais (1-16), tick/PPQ, meta eventos (time sig, key sig, tempo, EOT).
---

# midi-format

## Evento basico (status byte)

- `0x90..0x9F` Note On (channel 1..16); bytes: note, velocity.
- `0x80..0x8F` Note Off.
- `0xB0..0xBF` Continuous Controller (CC).
- `0xC0..0xCF` Program Change (PC).
- `0xE0..0xEF` Pitch Bend (14-bit, lsb/msb).
- `0xF0...` Meta/system messages.

## Ticks & PPQ

- **PPQ** (pulses per quarter): padrao DAW = 480. Music Theory Engine usa 480.
- Tempo (BPM) define quarter duration: quarter = (60/BPM) seconds.
- Tempo map em MIDI via meta `0xFF 0x51 0x03` (set tempo, microseconds/quarter).

## Meta eventos importantes

- `0xFF 0x51` Tempo (set tempo).
- `0xFF 0x58` Time signature.
- `0xFF 0x59` Key signature.
- `0xFF 0x2F` End of Track.
- `0xFF 0x06` Text marker (usado para sections A/B/C...).

## No projeto

- Tudo em ticks (PPQ 480) interno; conversao para samples no Plugin.
- Multiplos canais conforme instrument-mapper (1 tambor, 2 bass, 3 piano...).
