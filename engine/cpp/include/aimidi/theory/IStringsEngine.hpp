#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <memory>
#include <vector>
#include <string_view>

namespace aimidi::theory {

// String section type.
enum class StringsSection { violin, viola, cello, contrabass, full_ensemble };

// StringsEngine: generates realistic string section MIDI.
class IStringsEngine {
public:
    virtual ~IStringsEngine() = default;

    // Generate string pad/harmony for a chord progression.
    virtual std::vector<MidiEvent> generate_pad(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const = 0;

    // Generate string countermelody.
    virtual std::vector<MidiEvent> generate_countermelody(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const = 0;

    // Generate string swells (crescendo on held notes).
    virtual std::vector<MidiEvent> generate_swells(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const = 0;
};

std::unique_ptr<IStringsEngine> make_strings_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords);

} // namespace aimidi::theory
