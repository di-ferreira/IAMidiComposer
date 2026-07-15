#include <aimidi/theory/IReharmonizeEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(ReharmonizeEngineTest, CreatesEngine) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    ASSERT_TRUE(engine);
}

TEST(ReharmonizeEngineTest, SimpleReharm) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    ReharmonizeRequest req{};
    req.melody = {{0, 480, 0, 60, 100}, {480, 480, 0, 64, 100}, {960, 480, 0, 67, 100}};
    req.key = "C";
    req.scale = "major";
    req.original_chords = {"C", "F", "G"};
    req.bars = 2;
    req.bpm = 120;
    req.style = ReharmStyle::simple;
    req.seed = 42;
    auto chords_out = engine->reharmonize(req);
    EXPECT_FALSE(chords_out.empty());
}

TEST(ReharmonizeEngineTest, JazzReharm) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    ReharmonizeRequest req{};
    req.melody = {{0, 480, 0, 60, 100}, {480, 480, 0, 64, 100}, {960, 480, 0, 67, 100}};
    req.key = "C";
    req.scale = "major";
    req.original_chords = {"C", "F", "G"};
    req.bars = 2;
    req.bpm = 120;
    req.style = ReharmStyle::jazz;
    req.seed = 42;
    auto chords_out = engine->reharmonize(req);
    EXPECT_FALSE(chords_out.empty());
}

TEST(ReharmonizeEngineTest, DifferentFromOriginal) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    ReharmonizeRequest req{};
    req.melody = {{0, 480, 0, 60, 100}, {480, 480, 0, 64, 100}, {960, 480, 0, 67, 100}};
    req.key = "C";
    req.scale = "major";
    req.original_chords = {"C", "F", "G"};
    req.bars = 2;
    req.bpm = 120;
    req.style = ReharmStyle::jazz;
    req.seed = 42;
    auto new_chords = engine->reharmonize(req);
    bool different = false;
    for (std::size_t i = 0; i < new_chords.size() && i < 3; ++i) {
        if (new_chords[i] != req.original_chords[i]) different = true;
    }
    EXPECT_TRUE(different);
}

TEST(ReharmonizeEngineTest, Voicings) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    auto events = engine->generate_voicings({"Cmaj7", "Fmaj7", "G7", "Cmaj7"}, 2, 120, 42);
    EXPECT_FALSE(events.empty());
}

TEST(ReharmonizeEngineTest, SeedDeterministic) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    auto harmony = std::shared_ptr<IHarmonyEngine>(
        make_harmony_engine(scales, chords).release());
    auto engine = make_reharmonize_engine(scales, chords, harmony);
    ReharmonizeRequest req{};
    req.melody = {{0, 480, 0, 60, 100}, {480, 480, 0, 64, 100}, {960, 480, 0, 67, 100}};
    req.key = "C";
    req.scale = "major";
    req.original_chords = {"C", "F", "G"};
    req.bars = 2;
    req.bpm = 120;
    req.style = ReharmStyle::simple;
    req.seed = 42;
    auto a = engine->reharmonize(req);
    auto b = engine->reharmonize(req);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i], b[i]);
    }
}
