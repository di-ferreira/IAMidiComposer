---
name: midi-event
description: Modelo interno do projeto para eventos MIDI - struct {tick, duracao, channel, note, velocity, articulacao}; aderente a PPQ 480.
---

# midi-event

## Estrutura (C++)

```cpp
struct MidiEvent {
    uint32_t   tick_on;       // PPQ 480 unidades
    uint32_t   tick_off;
    uint8_t    channel;       // 0..15 (display +1)
    uint8_t    note;          // 0..127 (pitches)
    uint8_t    velocity;      // 0..127
    Articulation articulation; // note / stacc / legato / accent / palm_mute /...
};
```

## Tipos addicazes

```cpp
struct CCEvent { uint32_t tick; uint8_t channel; uint8_t controller; uint8_t value; };
struct PCEvent { uint32_t tick; uint8_t channel; uint8_t program; };
struct PitchBend { uint32_t tick; uint8_t channel; int16_t bend; };
struct MetaMarker { uint32_t tick; std::string_view name; };
```

## Anti-padroes

- `std::string` por evento (copia/alocação; use arena + std::string_view).
- Heranca e virtual no path critico; use variant/struct-of-arrays.
- Polimorfismo quando a equivalência estrutural basta.

## Layout preferido

- SOA arrays por tipo quando em processamento batch (SIMD/cache).
- AOS em virar MIDI bytes (raramente durante render).
