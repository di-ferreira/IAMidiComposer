#include <aimidi/theory/IOrchestrateWorkflow.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <algorithm>

using namespace aimidi::theory;

namespace {

std::unique_ptr<IOrchestrateWorkflow> make_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto orch   = make_orchestration_engine(scales, chords);
    auto str    = make_strings_engine(scales, chords);
    auto gtr    = make_guitar_engine(scales, chords);
    auto bass   = make_bass_engine();
    auto drums  = make_drum_engine();
    return make_orchestrate_workflow(
        std::move(orch), std::move(str), std::move(gtr),
        std::move(bass), std::move(drums));
}

// Build a simple C major piano idea: melody notes + chord tones.
std::vector<MidiEvent> make_cmajor_piano() {
    std::vector<MidiEvent> out;

    // Bar 1: C major chord (C4+E4+G4) with melody C5 on top
    out.push_back({   0u, 460u, 0, 60, 100, 0u});  // C4  (harmony)
    out.push_back({   0u, 460u, 0, 64, 100, 0u});  // E4  (harmony)
    out.push_back({   0u, 460u, 0, 67, 100, 0u});  // G4  (harmony)
    out.push_back({   0u, 460u, 0, 72, 100, 0u});  // C5  (melody)

    // Beat 2: melody moves to D5, chord holds
    out.push_back({ 480u, 920u, 0, 60, 100, 0u});  // C4  (harmony)
    out.push_back({ 480u, 920u, 0, 64, 100, 0u});  // E4  (harmony)
    out.push_back({ 480u, 920u, 0, 67, 100, 0u});  // G4  (harmony)
    out.push_back({ 480u, 920u, 0, 74, 100, 0u});  // D5  (melody)

    // Bar 2: F major chord (F3+A3+C4) with melody A4
    out.push_back({1920u, 2380u, 0, 53, 100, 0u});  // F3  (harmony)
    out.push_back({1920u, 2380u, 0, 57, 100, 0u});  // A3  (harmony)
    out.push_back({1920u, 2380u, 0, 60, 100, 0u});  // C4  (harmony)
    out.push_back({1920u, 2380u, 0, 69, 100, 0u});  // A4  (melody)

    // Beat 2: melody moves to G4
    out.push_back({2400u, 2840u, 0, 53, 100, 0u});  // F3  (harmony)
    out.push_back({2400u, 2840u, 0, 57, 100, 0u});  // A3  (harmony)
    out.push_back({2400u, 2840u, 0, 60, 100, 0u});  // C4  (harmony)
    out.push_back({2400u, 2840u, 0, 67, 100, 0u});  // G4  (melody)

    return out;
}

} // namespace

TEST(OrchestrateWorkflowTest, CreatesEngine) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
}

TEST(OrchestrateWorkflowTest, PianoToOrchestra) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = make_cmajor_piano();
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::medium;
    req.seed = 42;

    auto result = engine->orchestrate(req);
    EXPECT_FALSE(result.empty());
    // At least 3 tracks: melody, harmony, bass
    EXPECT_GE(result.size(), 3u);
}

TEST(OrchestrateWorkflowTest, SparseDensity) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = make_cmajor_piano();
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::sparse;
    req.seed = 99;

    auto result = engine->orchestrate(req);
    EXPECT_FALSE(result.empty());
    // Sparse: strings + bass only, no drums
    bool has_drums = false;
    for (const auto& [name, events] : result) {
        if (name == "Drums") { has_drums = true; break; }
    }
    EXPECT_FALSE(has_drums);
}

TEST(OrchestrateWorkflowTest, FullDensity) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = make_cmajor_piano();
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::full;
    req.seed = 77;

    auto result = engine->orchestrate(req);
    EXPECT_FALSE(result.empty());
    // Full should have more tracks than sparse
    EXPECT_GE(result.size(), 4u);
}

TEST(OrchestrateWorkflowTest, SeedDeterministic) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = make_cmajor_piano();
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::medium;
    req.seed = 42;

    auto a = engine->orchestrate(req);
    auto b = engine->orchestrate(req);

    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].first, b[i].first);
        ASSERT_EQ(a[i].second.size(), b[i].second.size());
        for (std::size_t j = 0; j < a[i].second.size(); ++j) {
            EXPECT_EQ(a[i].second[j].tick_on,  b[i].second[j].tick_on);
            EXPECT_EQ(a[i].second[j].note,     b[i].second[j].note);
            EXPECT_EQ(a[i].second[j].velocity, b[i].second[j].velocity);
        }
    }
}

TEST(OrchestrateWorkflowTest, EmptyInputReturnsEmpty) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = {};
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::medium;
    req.seed = 0;

    auto result = engine->orchestrate(req);
    EXPECT_TRUE(result.empty());
}

TEST(OrchestrateWorkflowTest, SingleNotePiano) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = {{0u, 480u, 0, 60, 100, 0u}};  // Single C4
    req.key  = "C";
    req.scale = "major";
    req.bars = 1;
    req.bpm  = 120;
    req.density = OrchestrateDensity::medium;
    req.seed = 1;

    auto result = engine->orchestrate(req);
    EXPECT_FALSE(result.empty());
}

TEST(OrchestrateWorkflowTest, TracksHaveValidNotes) {
    auto engine = make_engine();
    OrchestrateRequest req;
    req.piano_input = make_cmajor_piano();
    req.key  = "C";
    req.scale = "major";
    req.bars = 2;
    req.bpm  = 120;
    req.density = OrchestrateDensity::medium;
    req.seed = 42;

    auto result = engine->orchestrate(req);
    for (const auto& [name, events] : result) {
        for (const auto& ev : events) {
            EXPECT_LE(ev.note, 127u);
            EXPECT_GT(ev.tick_off, ev.tick_on);
            EXPECT_GE(ev.velocity, 1u);
            EXPECT_LE(ev.velocity, 127u);
        }
    }
}
