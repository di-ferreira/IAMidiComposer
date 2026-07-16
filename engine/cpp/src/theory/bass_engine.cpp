// Implementation: BassEngine — seed-deterministic bass line patterns.
#include <aimidi/core/Tracy.hpp>
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/Types.hpp>
#include <random>
#include <algorithm>

namespace aimidi::theory {

namespace {

enum Pattern { A, B, C, D };

constexpr int kBassRootMidi = 24;
constexpr int kBassMaxMidi  = 60;

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity, std::uint8_t channel) {
    MidiEvent e{};
    e.tick_on     = tick_on;
    e.tick_off    = tick_off;
    e.channel     = channel;
    e.note        = note;
    e.velocity    = velocity;
    e.articulation = 0u;
    return e;
}

int root_midi(int root_pc) {
    return kBassRootMidi + (root_pc % 12);
}

int fifth_midi(int root_pc) {
    return std::min(kBassRootMidi + root_pc + 7, kBassMaxMidi);
}

int walking_note(int root_pc, int choice, std::mt19937_64& rng) {
    int base = kBassRootMidi + (root_pc % 12);
    int note;
    switch (choice) {
        case 0: note = base;                          break;
        case 1: note = base + 2;                     break;
        case 2: note = base + 3;                     break;
        default: note = base + 7;                     break;
    }
    if (note < kBassRootMidi) note += 12;
    return std::min(note, kBassMaxMidi);
}

} // namespace

class BassEngine final : public IBassEngine {
public:
    std::vector<MidiEvent> generate(const ChordRequest& req, const BassStyle& style) const override {
        ZoneScoped;
        std::vector<MidiEvent> out;

        for (std::size_t i = 0; i < req.chords.size(); ++i) {
            const auto& cs = req.chords[i];
            std::mt19937_64 rng(req.seed ^ static_cast<std::uint64_t>(i));
            Pattern pat = static_cast<Pattern>(rng() % 4);

            const int ppq = static_cast<int>(req.ppq);
            const int root_dur = ppq - 10;
            const int fifth_dur = ppq / 2 - 5;

            std::uint8_t root_vel = static_cast<std::uint8_t>(50 + rng() % 51);
            std::uint8_t fifth_vel = static_cast<std::uint8_t>(50 + rng() % 26);

            switch (pat) {
                case Pattern::A: {
                    for (int beat = 0; beat < 4; ++beat) {
                        int tick = cs.start_tick + beat * ppq;
                        int note = root_midi(cs.root_pc);
                        out.push_back(make_note(tick, tick + root_dur, note, root_vel, 0));
                    }
                    break;
                }
                case Pattern::B: {
                    out.push_back(make_note(cs.start_tick,
                                            cs.start_tick + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq,
                                            cs.start_tick + ppq + fifth_dur,
                                            fifth_midi(cs.root_pc), fifth_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq * 2,
                                            cs.start_tick + ppq * 2 + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq * 3,
                                            cs.start_tick + ppq * 3 + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    break;
                }
                case Pattern::C: {
                    int w1 = walking_note(cs.root_pc, static_cast<int>(rng() % 4), rng);
                    int w3 = walking_note(cs.root_pc, static_cast<int>(rng() % 4), rng);
                    out.push_back(make_note(cs.start_tick,
                                            cs.start_tick + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq,
                                            cs.start_tick + ppq + root_dur,
                                            w1, root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq * 2,
                                            cs.start_tick + ppq * 2 + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq * 3,
                                            cs.start_tick + ppq * 3 + root_dur,
                                            w3, root_vel, 0));
                    break;
                }
                case Pattern::D: {
                    out.push_back(make_note(cs.start_tick,
                                            cs.start_tick + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq,
                                            cs.start_tick + ppq + fifth_dur,
                                            fifth_midi(cs.root_pc), fifth_vel, 0));
                    out.push_back(make_note(cs.start_tick + ppq * 2,
                                            cs.start_tick + ppq * 2 + root_dur,
                                            root_midi(cs.root_pc), root_vel, 0));
                    break;
                }
            }
        }
        return out;
    }
};

std::unique_ptr<IBassEngine> make_bass_engine() {
    return std::make_unique<BassEngine>();
}

} // namespace aimidi::theory