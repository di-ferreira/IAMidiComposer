// Benchmark: MTE end-to-end pipeline.
//
// Measures time to generate SMF from a MusicBlueprint through the full engine
// chain: harmony → chords → bass → rhythm → drums → piano → humanization →
// SMF render.
//
// Targets:
//   - HarmonyEngine 4 bars : <1 ms
//   - HarmonyEngine 16 bars: <5 ms
//   - Full pipeline 4 bars : <10 ms
//   - Full pipeline 16 bars: <50 ms
//   - M3 end-to-end        : <5 s for 16 bars (incl. AI, no GPU)
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/IRhythmEngine.hpp>
#include <aimidi/theory/IDrumEngine.hpp>
#include <aimidi/theory/IPianoEngine.hpp>
#include <aimidi/theory/IHumanizationEngine.hpp>
#include <aimidi/theory/IMidiRenderer.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <aimidi/theory/IBlueprintGenerator.hpp>
#include <aimidi/theory/ITimelinePlanner.hpp>
#include <aimidi/theory/IArrangementPlanner.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <vector>
#include <cstdint>

using namespace aimidi::theory;

namespace {

template<typename F>
double measure_ms(F&& f, int iterations = 10) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return static_cast<double>(duration.count()) / (1000.0 * iterations);
}

std::shared_ptr<IScaleProvider> shared_scales() {
    return std::shared_ptr<IScaleProvider>(make_scale_provider().release());
}

std::shared_ptr<IChordEngine> shared_chords() {
    return std::shared_ptr<IChordEngine>(make_chord_engine().release());
}

} // namespace

TEST(BenchmarkTest, HarmonyEngine4Bars) {
    auto harmony = make_harmony_engine(shared_scales(), shared_chords());
    ASSERT_TRUE(harmony);

    HarmonyRequest req{};
    req.root_key = "C";
    req.scale = "major";
    req.bars = 4;
    req.seed = 42;

    double ms = measure_ms([&]() {
        auto result = harmony->generate(req);
        ASSERT_FALSE(result.empty());
    }, 100);

    EXPECT_LT(ms, 1.0) << "HarmonyEngine 4 bars: " << ms << " ms (target <1ms)";
    std::cout << "[BENCH] HarmonyEngine 4 bars: " << ms << " ms" << std::endl;
}

TEST(BenchmarkTest, HarmonyEngine16Bars) {
    auto harmony = make_harmony_engine(shared_scales(), shared_chords());
    ASSERT_TRUE(harmony);

    HarmonyRequest req{};
    req.root_key = "C";
    req.scale = "major";
    req.bars = 16;
    req.seed = 42;

    double ms = measure_ms([&]() {
        auto result = harmony->generate(req);
        ASSERT_FALSE(result.empty());
    }, 50);

    EXPECT_LT(ms, 5.0) << "HarmonyEngine 16 bars: " << ms << " ms (target <5ms)";
    std::cout << "[BENCH] HarmonyEngine 16 bars: " << ms << " ms" << std::endl;
}

TEST(BenchmarkTest, FullPipeline4Bars) {
    auto harmony = make_harmony_engine(shared_scales(), shared_chords());
    auto bass = make_bass_engine();
    auto rhythm = make_rhythm_engine();
    auto drums = make_drum_engine();
    auto piano = make_piano_engine();
    auto humanization = make_humanization_engine();
    auto renderer = make_midi_renderer();

    ASSERT_TRUE(harmony);
    ASSERT_TRUE(bass);
    ASSERT_TRUE(rhythm);
    ASSERT_TRUE(drums);
    ASSERT_TRUE(piano);
    ASSERT_TRUE(humanization);
    ASSERT_TRUE(renderer);

    double ms = measure_ms([&]() {
        HarmonyRequest hreq{};
        hreq.root_key = "C";
        hreq.scale = "major";
        hreq.bars = 4;
        hreq.seed = 42;
        auto harmony_events = harmony->generate(hreq);

        ChordRequest creq{};
        creq.seed = 42;
        creq.ppq = static_cast<std::uint32_t>(kPpq);
        for (std::size_t i = 0; i < harmony_events.size(); ++i) {
            creq.chords.push_back({
                static_cast<std::uint32_t>(i) * kPpq * 4,
                kPpq * 4,
                harmony_events[i].note % 12,
                "maj",
                0
            });
        }

        BassStyle bstyle{};
        auto bass_events = bass->generate(creq, bstyle);

        RhythmStyle rstyle{};
        FillSpec fill{};
        auto rhythm_events = rhythm->generate(creq, rstyle, fill);

        DrumStyle dstyle{};
        auto drum_events = drums->generate(creq, dstyle);

        PianoRequest preq{};
        preq.chord_req = creq;
        preq.style = PianoStyle::block_chords;
        auto piano_events = piano->generate(preq);

        HumanizationParams hparams{};
        hparams.seed = 42;
        hparams.timing_jitter_ticks = 3;
        hparams.velocity_variation = 5;
        humanization->apply(piano_events, hparams);
        humanization->apply(bass_events, hparams);
        humanization->apply(rhythm_events, hparams);
        humanization->apply(drum_events, hparams);

        SmfComposition comp{};
        comp.ppq = kPpq;
        comp.bpm = 120;
        comp.key = "C";
        comp.scale = "major";
        comp.tracks.push_back({"Piano", piano_events});
        comp.tracks.push_back({"Bass", bass_events});
        comp.tracks.push_back({"Drums", rhythm_events});
        for (auto& e : drum_events) {
            comp.tracks[2].events.push_back(e);
        }
        auto smf = renderer->render(comp);
        ASSERT_FALSE(smf.empty());
    }, 20);

    EXPECT_LT(ms, 10.0) << "Full pipeline 4 bars: " << ms << " ms (target <10ms)";
    std::cout << "[BENCH] Full pipeline 4 bars: " << ms << " ms" << std::endl;
}

TEST(BenchmarkTest, FullPipeline16Bars) {
    auto harmony = make_harmony_engine(shared_scales(), shared_chords());
    auto bass = make_bass_engine();
    auto rhythm = make_rhythm_engine();
    auto drums = make_drum_engine();
    auto piano = make_piano_engine();
    auto humanization = make_humanization_engine();
    auto renderer = make_midi_renderer();

    ASSERT_TRUE(harmony);
    ASSERT_TRUE(bass);
    ASSERT_TRUE(rhythm);
    ASSERT_TRUE(drums);
    ASSERT_TRUE(piano);
    ASSERT_TRUE(humanization);
    ASSERT_TRUE(renderer);

    double ms = measure_ms([&]() {
        HarmonyRequest hreq{};
        hreq.root_key = "C";
        hreq.scale = "major";
        hreq.bars = 16;
        hreq.seed = 42;
        auto harmony_events = harmony->generate(hreq);

        ChordRequest creq{};
        creq.seed = 42;
        creq.ppq = static_cast<std::uint32_t>(kPpq);
        for (std::size_t i = 0; i < harmony_events.size(); ++i) {
            creq.chords.push_back({
                static_cast<std::uint32_t>(i) * kPpq * 4,
                kPpq * 4,
                harmony_events[i].note % 12,
                "maj",
                0
            });
        }

        BassStyle bstyle{};
        auto bass_events = bass->generate(creq, bstyle);

        RhythmStyle rstyle{};
        FillSpec fill{};
        auto rhythm_events = rhythm->generate(creq, rstyle, fill);

        DrumStyle dstyle{};
        auto drum_events = drums->generate(creq, dstyle);

        PianoRequest preq{};
        preq.chord_req = creq;
        preq.style = PianoStyle::block_chords;
        auto piano_events = piano->generate(preq);

        HumanizationParams hparams{};
        hparams.seed = 42;
        hparams.timing_jitter_ticks = 3;
        hparams.velocity_variation = 5;
        humanization->apply(piano_events, hparams);
        humanization->apply(bass_events, hparams);
        humanization->apply(rhythm_events, hparams);
        humanization->apply(drum_events, hparams);

        SmfComposition comp{};
        comp.ppq = kPpq;
        comp.bpm = 120;
        comp.key = "C";
        comp.scale = "major";
        comp.tracks.push_back({"Piano", piano_events});
        comp.tracks.push_back({"Bass", bass_events});
        comp.tracks.push_back({"Drums", rhythm_events});
        for (auto& e : drum_events) {
            comp.tracks[2].events.push_back(e);
        }
        auto smf = renderer->render(comp);
        ASSERT_FALSE(smf.empty());
    }, 10);

    EXPECT_LT(ms, 50.0) << "Full pipeline 16 bars: " << ms << " ms (target <50ms)";
    std::cout << "[BENCH] Full pipeline 16 bars: " << ms << " ms" << std::endl;
}
