// Music Theory Engine — common types.
#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace aimidi::theory {

// PPQ (pulses per quarter note) used everywhere in the project.
inline constexpr int kPpq = 480;

// Note pitch 0..127 (MIDI clave).
enum class Note : std::uint8_t {};

// Velocity 0..127.
enum class Velocity : std::uint8_t {};

// Musical event in PPQ 480 ticks. SOA-friendly POD.
struct MidiEvent {
    std::uint32_t tick_on;
    std::uint32_t tick_off;
    std::uint8_t  channel;
    std::uint8_t  note;
    std::uint8_t  velocity;
    std::uint8_t  articulation;

    constexpr bool operator==(const MidiEvent& other) const noexcept {
        return tick_on == other.tick_on
            && tick_off == other.tick_off
            && channel == other.channel
            && note == other.note
            && velocity == other.velocity
            && articulation == other.articulation;
    }
};

// Reproducible seed type.
using Seed = std::uint64_t;

// Lookup for note name ( MIDI note -> "C4", "C#4", ... ).
constexpr std::string_view note_name(Note n) noexcept;

} // namespace aimidi::theory
