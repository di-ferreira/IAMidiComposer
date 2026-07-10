// Implementation: Harmony Engine — vertical spike (pop/rock 4/4, C major).
//
// Phase 2.1 (Sprint 4 / Milestone M2): emits real musical MIDI for the first
// time. The engine builds a II-V-I(-I) progressão per 4-bar section,
// translates to ChordSpec, and delegates voicing to IChordEngine.
//
// Determinism: same seed + same request → identical bytes (std::mt19937_64).
// Bench target: <1ms for 4 sections of 4 bars (16 chords × ~4 notes).
//
// TODO (Sprint 5): full voice-leading, IPianoEngine, IPiano voicings.
// TODO (Sprint 6): SMF rendering via MIDIRenderEngine.
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>

#include <memory>
#include <random>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <array>

namespace aimidi::theory {

namespace {

// Pitch-class table for note names (C=0 .. B=11, enharmonics mapped).
// Bench target: <1ms for 4 sections of 4 bars (16 chords × ~4 notes).
const std::unordered_map<std::string_view, int>& key_table() {
    static const std::unordered_map<std::string_view, int> k = {
        {"C",0},  {"C#",1}, {"Db",1}, {"D",2},  {"D#",3}, {"Eb",3},
        {"E",4},  {"F",5},  {"F#",6}, {"Gb",6}, {"G",7},  {"G#",8},
        {"Ab",8}, {"A",9},  {"A#",10},{"Bb",10},{"B",11},
    };
    return k;
}

int parse_root_pc(std::string_view root_key) {
    const auto& k = key_table();
    const auto it = k.find(root_key);
    return it != k.end() ? it->second : 0; // default C if unknown
}

// Build a 4-bar chord set following II-V-I-I per section (C-based for Phase 2.1).
// Degree II = Dm7 (pc=2, "m7"), V = G7 (pc=7, "7"), I = Cmaj7 (pc=0, "maj7").
struct ChordSlot { int pc; const char* quality; };

constexpr std::array<ChordSlot, 4> kIIVIPattern = {{
    {2, "m7"},    // II (Dm7)
    {7, "7"},     // V  (G7)
    {0, "maj7"},  // I  (Cmaj7)
    {0, "maj7"},  // I  (Cmaj7) static resolution
}};

class HarmonyEngine final : public IHarmonyEngine {
public:
    HarmonyEngine(std::shared_ptr<IScaleProvider> scales,
                  std::shared_ptr<IChordEngine>   chords)
        : scales_(std::move(scales))
        , chords_(std::move(chords)) {}

    std::vector<MidiEvent> generate(const HarmonyRequest& req) const override {
        std::mt19937_64 rng(req.seed);
        (void)rng; // reserved for future per-bar jitter; deterministic.

        if (req.bars < 1) {
            return {};
        }

        const int root_pc = parse_root_pc(req.root_key);

        // Validate scale name; fall back to "major" if unknown.
        // Phase 2.1 uses the C-rooted II-V-I pattern regardless of key/scale.
        if (scales_) {
            auto sp = scales_->intervals_of(req.scale);
            if (sp.empty()) {
                sp = scales_->intervals_of("major");
            }
        }
        (void)root_pc;

        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u; // 4/4

        std::vector<ChordSpec> specs;
        specs.reserve(static_cast<std::size_t>(req.bars));
        for (int bar = 0; bar < req.bars; ++bar) {
            const auto& slot = kIIVIPattern[static_cast<std::size_t>(bar % 4)];
            ChordSpec cs{};
            cs.start_tick    = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            cs.duration_ticks= ticks_per_bar;
            cs.root_pc       = slot.pc;
            cs.quality       = slot.quality;
            cs.channel       = 0u;
            specs.push_back(std::move(cs));
        }

        if (!chords_) {
            return {};
        }
        return chords_->voicing(ChordRequest{std::move(specs), req.seed, ppq});
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
    std::shared_ptr<IChordEngine>   chords_;
};

} // namespace

// Factory: back-compatible — chords default-constructed if nullptr.
std::unique_ptr<IHarmonyEngine> make_harmony_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine>   chords) {
    if (!chords) {
        chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    }
    return std::make_unique<HarmonyEngine>(std::move(scales), std::move(chords));
}

// Overload kept for back-compat with existing callers.
std::unique_ptr<IHarmonyEngine> make_harmony_engine(
    std::shared_ptr<IScaleProvider> scales) {
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return std::make_unique<HarmonyEngine>(std::move(scales), std::move(chords));
}

} // namespace aimidi::theory
