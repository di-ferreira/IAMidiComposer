// Humanization engine interface.
//
// Applies seed-deterministic micro-timing, velocity variation, groove swing,
// and grid snap to a sequence of MidiEvents in-place.
#pragma once

#include <aimidi/theory/Types.hpp>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

// Style-specific humanization presets.
enum class HumanizationStyle { standard, swing, laid_back, pushed, snap_to_grid };

// Humanization parameters applied to a sequence of MidiEvents.
struct HumanizationParams {
    Seed  seed = 0;
    int   timing_jitter_ticks = 5;   // max +-ticks to shift tick_on/tick_off
    int   velocity_variation = 10;   // max +-velocity variation
    float groove_swing = 0.0f;       // 0=straight, 1=full swing (delays off-beat eighth notes)
    bool  snap_to_grid = false;      // if true, quantize to nearest grid after jitter
    int   grid_resolution = 120;     // grid in ticks (120 = 16th note at 480 ppq)
};

class IHumanizationEngine {
public:
    virtual ~IHumanizationEngine() = default;

    // Apply humanization to a vector of MidiEvents in-place.
    // Modifies tick_on, tick_off, and velocity according to params.
    // The seed drives all randomness deterministically.
    virtual void apply(std::vector<MidiEvent>& events, const HumanizationParams& params) const = 0;

    // Apply style-specific humanization as a convenience wrapper.
    // Returns a new vector (does not modify input).
    // The style controls timing feel and velocity profile:
    //   standard    — light jitter + velocity variation
    //   swing       — offbeat eighth-note swing (~30%)
    //   laid_back   — notes delayed + velocity reduced
    //   pushed      — notes advanced + velocity increased
    //   snap_to_grid — quantize to 16th-note grid, no variation
    virtual std::vector<MidiEvent> humanize_by_style(
        const std::vector<MidiEvent>& input,
        HumanizationStyle style,
        int bpm,
        Seed seed) const = 0;
};

std::unique_ptr<IHumanizationEngine> make_humanization_engine();
}
