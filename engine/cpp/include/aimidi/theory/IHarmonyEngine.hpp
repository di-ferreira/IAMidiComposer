// Harmony engine interface.
//
// Pure interface: receives a Music Blueprint (proto schema reference; for the
// skeleton stage we pass it as opaque pointer-friendly struct) plus a Seed,
// emits a sequence of chords as MidiEvents with tick container.
//
// Concrete engine (e.g. HarmonyEngine) is injected via DI.
#pragma once

#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <vector>
#include <memory>

namespace aimidi::theory {

struct HarmonyRequest {
    std::string_view root_key;   // e.g. "C"
    std::string_view scale;      // e.g. "major"
    int              bars = 4;
    Seed             seed = 0;
};

class IHarmonyEngine {
public:
    virtual ~IHarmonyEngine() = default;

    /// Generate a chord per bar; returns a vector of MidiEvents (each chord
    /// described as multiple notes starting at the same tick).
    virtual std::vector<MidiEvent> generate(const HarmonyRequest& req) const = 0;
};

/// Factory: builds a HarmonyEngine injecting a ScaleProvider (DI).
std::unique_ptr<IHarmonyEngine> make_harmony_engine(
    std::shared_ptr<IScaleProvider> scales);

} // namespace aimidi::theory
