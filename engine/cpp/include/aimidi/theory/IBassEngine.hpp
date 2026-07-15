// Bass engine interface.
#pragma once

#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <vector>
#include <memory>
#include <string_view>
#include <cstdint>

namespace aimidi::theory {

struct BassStyle {
    int  seed            = 0;
    bool use_octave_jump = false;
};

class IBassEngine {
public:
    virtual ~IBassEngine() = default;

    virtual std::vector<MidiEvent> generate(
        const ChordRequest& req,
        const BassStyle&     style) const = 0;
};

std::unique_ptr<IBassEngine> make_bass_engine();

} // namespace aimidi::theory