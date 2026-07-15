#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <memory>
#include <vector>
#include <string_view>

namespace aimidi::theory {

// Guitar playing style.
enum class GuitarStyle { strumming, arpeggio, power_chord, fingerpicking, riff, funk_chop };

// GuitarEngine: generates idiomatic guitar MIDI patterns.
class IGuitarEngine {
public:
    virtual ~IGuitarEngine() = default;

    // Generate guitar chords (strumming or arpeggio).
    virtual std::vector<MidiEvent> generate_chords(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, GuitarStyle style, Seed seed) const = 0;

    // Generate power chords (rock/metal).
    virtual std::vector<MidiEvent> generate_power_chords(
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, Seed seed) const = 0;

    // Generate a guitar riff from a scale.
    virtual std::vector<MidiEvent> generate_riff(
        std::string_view key, std::string_view scale,
        int bars, int bpm, Seed seed) const = 0;

    // Generate funk chops (short, syncopated chords).
    virtual std::vector<MidiEvent> generate_funk_chops(
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, Seed seed) const = 0;
};

std::unique_ptr<IGuitarEngine> make_guitar_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords);

} // namespace aimidi::theory
