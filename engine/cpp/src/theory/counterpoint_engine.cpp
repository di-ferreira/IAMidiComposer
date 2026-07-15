#include <aimidi/theory/ICounterpointEngine.hpp>
#include <aimidi/theory/Types.hpp>
#include <random>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdint>
#include <cstdlib>

namespace aimidi::theory {

namespace {

constexpr int kMinMidi = 60;
constexpr int kMaxMidi = 84;
constexpr int kConsonantIntervals[] = {0, 3, 4, 7, 8, 9};

constexpr bool is_consonant(int interval_class) noexcept {
    int ic = interval_class % 12;
    if (ic < 0) ic += 12;
    for (int c : kConsonantIntervals) {
        if (ic == c) return true;
    }
    return false;
}

std::vector<int> valid_midi_notes(int bass_pc, int min_midi, int max_midi) {
    std::vector<int> notes;
    int valid_pcs[6];
    for (int i = 0; i < 6; ++i) {
        valid_pcs[i] = ((bass_pc + kConsonantIntervals[i]) % 12 + 12) % 12;
    }
    for (int midi = min_midi; midi <= max_midi; ++midi) {
        int pc = ((midi % 12) + 12) % 12;
        for (int vpc : valid_pcs) {
            if (pc == vpc) {
                notes.push_back(midi);
                break;
            }
        }
    }
    return notes;
}

int interval_class(int counter_midi, int bass_pc) noexcept {
    return ((counter_midi - bass_pc) % 12 + 12) % 12;
}

int cf_direction(int prev, int curr) noexcept {
    if (curr > prev) return 1;
    if (curr < prev) return -1;
    return 0;
}

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity,
                    std::uint8_t channel) noexcept {
    MidiEvent e{};
    e.tick_on     = tick_on;
    e.tick_off    = tick_off;
    e.channel     = channel;
    e.note        = note;
    e.velocity    = velocity;
    e.articulation = 0u;
    return e;
}

int choose_velocity(std::mt19937_64& rng) noexcept {
    return static_cast<int>(60 + rng() % 41);
}

} // namespace

class CounterpointEngine final : public ICounterpointEngine {
public:
    explicit CounterpointEngine(std::shared_ptr<IScaleProvider> scales)
        : scales_(std::move(scales)) {}

    std::vector<MidiEvent> generate(const CounterpointRequest& req) const override {
        if (req.bass_notes.empty() || req.cantus_firmus.empty()) {
            return {};
        }

        std::mt19937_64 rng(req.seed);
        std::vector<MidiEvent> out;

        auto n = req.bass_notes.size();
        auto cf = req.cantus_firmus;
        auto cf_n = cf.size();
        int total_ticks = req.bars * req.ppq * 4;
        int slot_ticks = total_ticks / static_cast<int>(n);

        switch (req.species) {
            case CounterpointSpecies::first:
                generate_first_species(req, rng, out, n, cf, cf_n, slot_ticks);
                break;
            case CounterpointSpecies::second:
                generate_second_species(req, rng, out, n, cf, cf_n, slot_ticks);
                break;
        }

        return out;
    }

private:
    std::shared_ptr<IScaleProvider> scales_;

    void generate_first_species(
        const CounterpointRequest& req,
        std::mt19937_64& rng,
        std::vector<MidiEvent>& out,
        std::size_t n,
        const std::vector<int>& cf,
        std::size_t cf_n,
        int slot_ticks) const
    {
        int prev_note = -1;
        int prev_ic = -1;

        for (std::size_t i = 0; i < n; ++i) {
            int bass_pc = req.bass_notes[i];
            int tick = static_cast<int>(i) * slot_ticks;
            int dur = slot_ticks - 5;

            auto candidates = valid_midi_notes(bass_pc, kMinMidi, kMaxMidi);

            candidates = filter_first_species(
                candidates, bass_pc,
                i, n,
                prev_note, prev_ic,
                cf, cf_n);

            int chosen;
            if (candidates.empty()) {
                int octave = kMinMidi + ((bass_pc - (kMinMidi % 12) + 12) % 12);
                if (octave < kMinMidi) octave += 12;
                if (octave > kMaxMidi) octave -= 12;
                chosen = octave;
            } else {
                chosen = candidates[rng() % candidates.size()];
            }

            out.push_back(make_note(
                static_cast<std::uint32_t>(tick),
                static_cast<std::uint32_t>(tick + dur),
                static_cast<std::uint8_t>(chosen),
                static_cast<std::uint8_t>(choose_velocity(rng)),
                0));

            prev_note = chosen;
            prev_ic = interval_class(chosen, bass_pc);
        }
    }

    void generate_second_species(
        const CounterpointRequest& req,
        std::mt19937_64& rng,
        std::vector<MidiEvent>& out,
        std::size_t n,
        const std::vector<int>& cf,
        std::size_t cf_n,
        int slot_ticks) const
    {
        int prev_note = -1;
        int prev_ic = -1;
        int sub_slot = slot_ticks / 2;
        int dur = sub_slot - 5;

        for (std::size_t i = 0; i < n; ++i) {
            int bass_pc = req.bass_notes[i];
            int base_tick = static_cast<int>(i) * slot_ticks;

            for (int sub = 0; sub < 2; ++sub) {
                int tick = base_tick + sub * sub_slot;

                if (sub == 0) {
                    auto candidates = valid_midi_notes(bass_pc, kMinMidi, kMaxMidi);

                    candidates = filter_first_species(
                        candidates, bass_pc,
                        i, n,
                        prev_note, prev_ic,
                        cf, cf_n);

                    int chosen;
                    if (candidates.empty()) {
                        int octave = kMinMidi + ((bass_pc - (kMinMidi % 12) + 12) % 12);
                        if (octave < kMinMidi) octave += 12;
                        if (octave > kMaxMidi) octave -= 12;
                        chosen = octave;
                    } else {
                        chosen = candidates[rng() % candidates.size()];
                    }

                    out.push_back(make_note(
                        static_cast<std::uint32_t>(tick),
                        static_cast<std::uint32_t>(tick + dur),
                        static_cast<std::uint8_t>(chosen),
                        static_cast<std::uint8_t>(choose_velocity(rng)),
                        0));

                    prev_note = chosen;
                    prev_ic = interval_class(chosen, bass_pc);
                } else {
                    std::vector<int> passing;
                    int direction = (rng() % 2 == 0) ? 1 : -1;

                    for (int step = 1; step <= 3; ++step) {
                        int pt = prev_note + direction * step;
                        if (pt >= kMinMidi && pt <= kMaxMidi) {
                            if (is_consonant(interval_class(pt, bass_pc))) {
                                passing.push_back(pt);
                            } else {
                                int pt_pc = ((pt % 12) + 12) % 12;
                                int bass_pc_norm = ((bass_pc % 12) + 12) % 12;
                                int diff = (pt_pc - bass_pc_norm + 12) % 12;
                                if (diff == 1 || diff == 2 || diff == 5) {
                                    passing.push_back(pt);
                                }
                            }
                        }
                    }

                    int chosen;
                    if (passing.empty()) {
                        auto fallback = valid_midi_notes(bass_pc, kMinMidi, kMaxMidi);
                        if (fallback.empty()) {
                            int octave = kMinMidi + ((bass_pc - (kMinMidi % 12) + 12) % 12);
                            if (octave < kMinMidi) octave += 12;
                            if (octave > kMaxMidi) octave -= 12;
                            chosen = octave;
                        } else {
                            chosen = fallback[rng() % fallback.size()];
                        }
                    } else {
                        chosen = passing[rng() % passing.size()];
                    }

                    out.push_back(make_note(
                        static_cast<std::uint32_t>(tick),
                        static_cast<std::uint32_t>(tick + dur),
                        static_cast<std::uint8_t>(chosen),
                        static_cast<std::uint8_t>(choose_velocity(rng)),
                        0));

                    prev_note = chosen;
                    prev_ic = interval_class(chosen, bass_pc);
                }
            }
        }
    }

    std::vector<int> filter_first_species(
        std::vector<int> candidates,
        int bass_pc,
        std::size_t i,
        std::size_t n,
        int prev_note,
        int prev_ic,
        const std::vector<int>& cf,
        std::size_t cf_n) const
    {
        if (candidates.empty()) return candidates;

        if (i == 0 || i == n - 1) {
            std::vector<int> perfect;
            for (int c : candidates) {
                int ic = interval_class(c, bass_pc);
                if (ic == 0 || ic == 12) {
                    perfect.push_back(c);
                }
            }
            if (!perfect.empty()) {
                candidates = std::move(perfect);
            }
            return candidates;
        }

        if (prev_note >= 0) {
            std::vector<int> filtered;
            int cf_dir = 0;
            if (i < cf_n && i > 0) {
                cf_dir = cf_direction(cf[i - 1], cf[i]);
            }

            for (int c : candidates) {
                int ic = interval_class(c, bass_pc);
                if ((prev_ic == 7 && ic == 7) || (prev_ic == 0 && ic == 0)) {
                    continue;
                }
                if (cf_dir != 0) {
                    int cp_dir = cf_direction(prev_note, c);
                    if (cp_dir == -cf_dir) {
                        filtered.push_back(c);
                    }
                } else {
                    filtered.push_back(c);
                }
            }

            if (!filtered.empty()) {
                candidates = std::move(filtered);
            }
        }

        return candidates;
    }
};

std::unique_ptr<ICounterpointEngine> make_counterpoint_engine(
    std::shared_ptr<IScaleProvider> scales)
{
    return std::make_unique<CounterpointEngine>(std::move(scales));
}

} // namespace aimidi::theory
