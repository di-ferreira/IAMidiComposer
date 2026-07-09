---
name: midi-gm-mapping
description: General MIDI (GM) standard - 128 programas por canal, note mapping, drum set no canal 10. Mapa utilizado por defaiuto em ausência de VSTi-emergido.
---

# midi-gm-mapping

## Program numbers (1-128 = 0-127)

- 1-8: Piano (Acoustic Grand..Clavinet)
- 9-16: Chromatic Percussion (Celesta..Kalimba)
- 17-24: Organ (Rock Organ..)
- 25-32: Guitar (Nylon..Distortion Guitar)
- 33-40: Bass (Acoustic Bass..Synth Bass 2)
- 41-48: Strings/Ensemble
- 49-56: Brass
- 57-64: Reed/Pipe
- 65-72: Synth Lead
- 73-80: Synth Pad
- ...
- 113-120: Percussive
- 121-128: Sound Effects

## Drums

- **Canal 10 (9 zero-indexed)** + GM2 drum map:
  - 35/36:Bass Drum  · 38:Snare · 42:Closed HH · 46:Open HH · 49:Crash · 51:Ride.

## No projeto

- Por padrao, Music Theory Engine emite MIDI GM-mapped (DR=10 channel, brasas in 1).
- Quando o usuario tem VSTi próprio, o instrument-mapper reconfigura o canal e
  programa correspondente.
