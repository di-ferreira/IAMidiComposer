// Implementation: HumanizationEngine — seed-deterministic micro-timing, velocity
// variation, groove swing, and grid snap.
//
// Determinism: same seed & input -> identical output (std::mt19937_64).
#include <aimidi/theory/IHumanizationEngine.hpp>
#include <random>
#include <algorithm>
#include <cstdint>

namespace aimidi::theory {

namespace {

// Clamp a signed int to [lo, hi].
int clamp(int v, int lo, int hi) noexcept {
    return std::max(lo, std::min(v, hi));
}

// Round tick to nearest multiple of resolution.
std::uint32_t snap_tick(std::uint32_t tick, int resolution) noexcept {
    if (resolution <= 0) return tick;
    const int half = resolution / 2;
    return static_cast<std::uint32_t>(
        (static_cast<int>(tick) + half) / resolution * resolution);
}

} // namespace

class HumanizationEngine final : public IHumanizationEngine {
public:
    void apply(std::vector<MidiEvent>& events, const HumanizationParams& params) const override {
        if (events.empty()) return;

        std::mt19937_64 rng(params.seed);

        const int swing_delay = params.groove_swing > 0.0f
                                    ? static_cast<int>(params.groove_swing * kPpq / 4)
                                    : 0;

        for (auto& ev : events) {
            // --- 1. Groove swing (based on original position) ---
            if (swing_delay > 0 && (ev.tick_on % static_cast<std::uint32_t>(kPpq)) == static_cast<std::uint32_t>(kPpq / 2)) {
                ev.tick_on += static_cast<std::uint32_t>(swing_delay);
                ev.tick_off += static_cast<std::uint32_t>(swing_delay);
            }

            // --- 2. Timing jitter ---
            const int jitter = static_cast<int>(
                rng() % (2u * static_cast<std::uint32_t>(params.timing_jitter_ticks) + 1u))
                - params.timing_jitter_ticks;

            if (jitter != 0) {
                int new_on  = static_cast<int>(ev.tick_on)  + jitter;
                int new_off = static_cast<int>(ev.tick_off) + jitter;
                ev.tick_on  = static_cast<std::uint32_t>(std::max(0, new_on));
                ev.tick_off = static_cast<std::uint32_t>(std::max(new_on + 1, new_off));
            }

            // --- 3. Velocity variation ---
            const int v_delta = static_cast<int>(
                rng() % (2u * static_cast<std::uint32_t>(params.velocity_variation) + 1u))
                - params.velocity_variation;
            ev.velocity = static_cast<std::uint8_t>(
                clamp(static_cast<int>(ev.velocity) + v_delta, 1, 127));

            // --- Invariant: tick_off > tick_on ---
            if (ev.tick_off <= ev.tick_on) {
                ev.tick_off = ev.tick_on + 1;
            }
        }

        // --- 4. Snap to grid (after all other transformations) ---
        if (params.snap_to_grid && params.grid_resolution > 0) {
            const int res = params.grid_resolution;
            for (auto& ev : events) {
                ev.tick_on  = snap_tick(ev.tick_on,  res);
                ev.tick_off = snap_tick(ev.tick_off, res);
                if (ev.tick_off <= ev.tick_on) {
                    ev.tick_off = ev.tick_on + static_cast<std::uint32_t>(res);
                }
            }
        }
    }

    std::vector<MidiEvent> humanize_by_style(
        const std::vector<MidiEvent>& input,
        HumanizationStyle style,
        int bpm,
        Seed seed) const override
    {
        (void)bpm;
        if (input.empty()) return {};

        std::vector<MidiEvent> out = input;

        HumanizationParams params;
        params.seed                = seed;
        params.timing_jitter_ticks = 0;
        params.velocity_variation  = 0;
        params.groove_swing        = 0.0f;
        params.snap_to_grid        = false;
        params.grid_resolution     = 120;

        switch (style) {
            case HumanizationStyle::standard:
                params.timing_jitter_ticks = 5;
                params.velocity_variation  = 10;
                break;

            case HumanizationStyle::swing:
                // Swing: offbeat eighth notes delayed ~30% of eighth duration.
                params.timing_jitter_ticks = 3;
                params.velocity_variation  = 8;
                params.groove_swing = 0.30f;
                break;

            case HumanizationStyle::laid_back:
                // Laid-back: random jitter with a net-positive offset and
                // reduced velocity applied after the base pass.
                params.timing_jitter_ticks = 5;
                params.velocity_variation  = 8;
                break;

            case HumanizationStyle::pushed:
                // Pushed: random jitter with a net-negative offset and
                // increased velocity applied after the base pass.
                params.timing_jitter_ticks = 5;
                params.velocity_variation  = 8;
                break;

            case HumanizationStyle::snap_to_grid:
                params.snap_to_grid    = true;
                params.grid_resolution = 120;
                params.timing_jitter_ticks = 0;
                params.velocity_variation  = 0;
                break;
        }

        apply(out, params);

        // Style-specific post-processing.
        switch (style) {
            case HumanizationStyle::laid_back: {
                // Delay all notes by an additional ~10 ticks (≈10ms at 120 BPM).
                const int delay = 10;
                for (auto& ev : out) {
                    int new_on  = static_cast<int>(ev.tick_on)  + delay;
                    int new_off = static_cast<int>(ev.tick_off) + delay;
                    ev.tick_on  = static_cast<std::uint32_t>(std::max(0, new_on));
                    ev.tick_off = static_cast<std::uint32_t>(std::max(static_cast<int>(ev.tick_on) + 1, new_off));
                    // Reduce velocity by ~10%.
                    ev.velocity = static_cast<std::uint8_t>(
                        std::max(1, static_cast<int>(ev.velocity) * 9 / 10));
                }
                break;
            }
            case HumanizationStyle::pushed: {
                // Advance all notes by ~5 ticks (≈5ms at 120 BPM).
                const int advance = 5;
                for (auto& ev : out) {
                    int new_on  = static_cast<int>(ev.tick_on)  - advance;
                    int new_off = static_cast<int>(ev.tick_off) - advance;
                    ev.tick_on  = static_cast<std::uint32_t>(std::max(0, new_on));
                    ev.tick_off = static_cast<std::uint32_t>(std::max(static_cast<int>(ev.tick_on) + 1, new_off));
                    // Increase velocity by ~10%.
                    ev.velocity = static_cast<std::uint8_t>(
                        std::min(127, static_cast<int>(ev.velocity) * 11 / 10));
                }
                break;
            }
            default:
                break;
        }

        return out;
    }
};

std::unique_ptr<IHumanizationEngine> make_humanization_engine() {
    return std::make_unique<HumanizationEngine>();
}

} // namespace aimidi::theory
