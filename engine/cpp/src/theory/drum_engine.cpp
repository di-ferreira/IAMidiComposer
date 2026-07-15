#include <aimidi/theory/IDrumEngine.hpp>
#include <random>
#include <array>

namespace aimidi::theory {

namespace {
inline constexpr std::uint8_t kKick   = 36;
inline constexpr std::uint8_t kSnare  = 38;
inline constexpr std::uint8_t kClosedHH = 42;
inline constexpr std::uint8_t kOpenHH   = 46;
inline constexpr std::uint8_t kPedalHH  = 44;
inline constexpr std::uint8_t kCrash    = 49;
inline constexpr std::uint8_t kRide     = 51;
inline constexpr std::uint8_t kDrumChannel = 9;
inline constexpr std::uint32_t kNoteDuration = 60;

MidiEvent make_drum(std::uint32_t tick, std::uint8_t note, std::uint8_t velocity) {
    MidiEvent e{};
    e.tick_on     = tick;
    e.tick_off    = tick + kNoteDuration;
    e.channel     = kDrumChannel;
    e.note        = note;
    e.velocity    = velocity;
    e.articulation = 0u;
    return e;
}

class DrumEngine final : public IDrumEngine {
public:
    std::vector<MidiEvent> generate(const ChordRequest& req, const DrumStyle& style) const override {
        std::vector<MidiEvent> out;
        out.reserve(64);

        std::uint32_t bar_ticks = req.ppq * 4;

        for (std::size_t bar_idx = 0; bar_idx < req.chords.size(); ++bar_idx) {
            const auto& chord = req.chords[bar_idx];
            std::uint32_t bar_start = chord.start_tick;
            bool is_fill_bar = (bar_idx % 4 == 3);

            std::mt19937_64 rng(static_cast<std::uint64_t>(style.seed) ^ static_cast<std::uint64_t>(bar_idx));
            int variation = static_cast<int>(rng() % 4);

            if (is_fill_bar) {
                auto ev0 = make_drum(bar_start + 1440, kKick,   80);
                auto ev1 = make_drum(bar_start + 1560, kSnare,  75);
                auto ev2 = make_drum(bar_start + 1680, kKick,   80);
                auto ev3 = make_drum(bar_start + 1800, kSnare,  75);
                out.push_back(ev0);
                out.push_back(ev1);
                out.push_back(ev2);
                out.push_back(ev3);
                continue;
            }

            auto add_hh = [&](std::uint32_t tick, bool open) {
                std::uint8_t hh_note = open ? kOpenHH : kClosedHH;
                std::uint8_t vel = static_cast<std::uint8_t>(60 + (rng() % 20));
                out.push_back(make_drum(tick, hh_note, vel));
            };

            auto add_kick = [&](std::uint32_t tick, std::uint8_t vel) {
                out.push_back(make_drum(tick, kKick, vel));
            };

            auto add_snare = [&](std::uint32_t tick, std::uint8_t vel) {
                out.push_back(make_drum(tick, kSnare, vel));
            };

            auto add_ghost_snare = [&](std::uint32_t tick) {
                if (style.use_ghosts && rng() % 2 == 0) {
                    std::uint8_t ghost_vel = static_cast<std::uint8_t>(25 + (rng() % 11));
                    out.push_back(make_drum(tick, kSnare, ghost_vel));
                }
            };

            switch (variation) {
                case 0: {
                    add_kick(bar_start + 0, 100);
                    add_hh(bar_start + 0, false);
                    add_snare(bar_start + 480, 90);
                    add_hh(bar_start + 480, false);
                    add_ghost_snare(bar_start + 600);
                    add_kick(bar_start + 960, 95);
                    add_hh(bar_start + 960, false);
                    add_snare(bar_start + 1440, 90);
                    add_hh(bar_start + 1440, false);
                    add_ghost_snare(bar_start + 1560);
                    break;
                }
                case 1: {
                    add_kick(bar_start + 0, 100);
                    add_hh(bar_start + 0, false);
                    add_snare(bar_start + 480, 90);
                    add_hh(bar_start + 480, false);
                    add_ghost_snare(bar_start + 600);
                    add_kick(bar_start + 960, 95);
                    add_hh(bar_start + 960, false);
                    add_kick(bar_start + 1200, 80);
                    add_snare(bar_start + 1440, 90);
                    add_hh(bar_start + 1440, false);
                    add_ghost_snare(bar_start + 1560);
                    break;
                }
                case 2: {
                    add_kick(bar_start + 0, 100);
                    add_hh(bar_start + 0, false);
                    add_snare(bar_start + 480, 90);
                    add_hh(bar_start + 480, false);
                    add_ghost_snare(bar_start + 600);
                    add_kick(bar_start + 960, 95);
                    add_snare(bar_start + 720, 70);
                    add_hh(bar_start + 960, false);
                    add_snare(bar_start + 1440, 90);
                    add_hh(bar_start + 1440, false);
                    add_ghost_snare(bar_start + 1560);
                    break;
                }
                case 3: {
                    add_kick(bar_start + 0, 100);
                    add_hh(bar_start + 0, false);
                    add_snare(bar_start + 480, 90);
                    add_hh(bar_start + 480, false);
                    add_ghost_snare(bar_start + 600);
                    add_kick(bar_start + 960, 95);
                    add_hh(bar_start + 960, false);
                    add_snare(bar_start + 1440, 90);
                    add_hh(bar_start + 1440, style.use_open_hh);
                    add_ghost_snare(bar_start + 1560);
                    break;
                }
            }
        }
        return out;
    }
};
}

std::unique_ptr<IDrumEngine> make_drum_engine() {
    return std::make_unique<DrumEngine>();
}

}