#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace aimidi::theory {

// A modulation between two keys.
struct Modulation {
    std::string from_key;      // e.g. "C"
    std::string from_scale;    // e.g. "major"
    std::string to_key;        // e.g. "G"
    std::string to_scale;      // e.g. "major"
    int         pivot_bar;     // bar index where modulation occurs
    std::string pivot_chord;   // e.g. "G7" (the chord that bridges both keys)
    std::string type;          // "pivot", "direct", "enharmonic"
};

// ModulationEngine: handles key modulation between sections.
class IModulationEngine {
public:
    virtual ~IModulationEngine() = default;

    // Find a pivot chord between two keys.
    // Returns the chord name (e.g. "G7") or empty string if no pivot found.
    virtual std::string find_pivot_chord(
        std::string_view from_key, std::string_view from_scale,
        std::string_view to_key, std::string_view to_scale) const = 0;

    // Generate a modulation sequence between two keys.
    // Returns a list of chord descriptions (root_pc, quality) for the transition bars.
    virtual std::vector<std::pair<int, std::string>> generate_modulation(
        std::string_view from_key, std::string_view from_scale,
        std::string_view to_key, std::string_view to_scale,
        int transition_bars, Seed seed) const = 0;

    // Check if two keys are closely related (share many common chords).
    virtual bool are_closely_related(
        std::string_view key1, std::string_view scale1,
        std::string_view key2, std::string_view scale2) const = 0;

    // Get all closely related keys for a given key.
    virtual std::vector<std::string> closely_related_keys(
        std::string_view key, std::string_view scale) const = 0;
};

std::unique_ptr<IModulationEngine> make_modulation_engine(
    std::shared_ptr<IScaleProvider> scales);

} // namespace aimidi::theory
