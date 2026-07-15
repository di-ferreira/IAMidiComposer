#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <vector>
#include <memory>
#include <string_view>
#include <cstdint>

namespace aimidi::theory {

struct DrumStyle {
    int  seed          = 0;
    bool use_ghosts    = true;
    bool use_open_hh   = false;
};

class IDrumEngine {
public:
    virtual ~IDrumEngine() = default;

    virtual std::vector<MidiEvent> generate(
        const ChordRequest& req,
        const DrumStyle&     style) const = 0;
};

std::unique_ptr<IDrumEngine> make_drum_engine();
}