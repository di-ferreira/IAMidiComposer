// MIDI Renderer interface.
//
// Converts a structured SmfComposition (tick-based MidiEvents) into a complete
// Standard MIDI File (SMF Type 1) byte vector.
//
// Pure C++20, no external dependencies.
#pragma once

#include <aimidi/theory/Types.hpp>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <string_view>

namespace aimidi::theory {

// A single track in the SMF output.
struct MidiTrack {
    std::string              name;
    std::vector<MidiEvent>   events;
};

// Complete SMF composition metadata + tracks.
struct SmfComposition {
    int                     ppq = 480;
    int                     bpm = 120;
    std::string             key = "C";
    std::string             scale = "major";
    int                     time_sig_num = 4;
    int                     time_sig_den = 4;
    std::vector<MidiTrack>  tracks;
};

class IMidiRenderer {
public:
    virtual ~IMidiRenderer() = default;

    // Render the composition to a Standard MIDI File (SMF Type 1) byte vector.
    virtual std::vector<std::uint8_t> render(const SmfComposition& comp) const = 0;
};

std::unique_ptr<IMidiRenderer> make_midi_renderer();
}
