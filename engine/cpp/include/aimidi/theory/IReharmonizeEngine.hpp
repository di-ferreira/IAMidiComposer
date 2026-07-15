#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IModulationEngine.hpp>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace aimidi::theory {

// Reharmonization style.
enum class ReharmStyle { simple, jazz, modal, chromatic, pedal };

// ReharmonizeRequest: what to reharmonize.
struct ReharmonizeRequest {
    std::vector<MidiEvent> melody;           // existing melody to preserve
    std::string_view       key;              // original key
    std::string_view       scale;            // original scale
    std::vector<std::string> original_chords; // original chord progression
    int                    bars;
    int                    bpm;
    ReharmStyle            style = ReharmStyle::simple;
    Seed                   seed = 0;
};

// ReharmonizeEngine: generates new harmony while preserving melody.
class IReharmonizeEngine {
public:
    virtual ~IReharmonizeEngine() = default;

    // Generate a new chord progression that works with the given melody.
    virtual std::vector<std::string> reharmonize(
        const ReharmonizeRequest& req) const = 0;

    // Generate new chord voicings for the reharmonized progression.
    virtual std::vector<MidiEvent> generate_voicings(
        const std::vector<std::string>& new_chords,
        int bars, int bpm, Seed seed) const = 0;
};

std::unique_ptr<IReharmonizeEngine> make_reharmonize_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords,
    std::shared_ptr<IHarmonyEngine> harmony);

} // namespace aimidi::theory
