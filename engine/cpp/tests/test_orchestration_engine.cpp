#include <aimidi/theory/IOrchestrationEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace aimidi::theory;

namespace {

std::unique_ptr<IOrchestrationEngine> make_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return make_orchestration_engine(std::move(scales), std::move(chords));
}

} // namespace

TEST(OrchestrationEngineTest, CreatesEngine) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
}

TEST(OrchestrationEngineTest, SuggestEnsemble) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("jazz", 5);
    EXPECT_FALSE(ensemble.empty());
    int total = 0;
    for (const auto& s : ensemble) total += static_cast<int>(s.instruments.size());
    EXPECT_GE(total, 3);
    EXPECT_LE(total, 5);
}

TEST(OrchestrationEngineTest, SuggestStringQuartet) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("string quartet", 0);
    EXPECT_FALSE(ensemble.empty());
    int total = 0;
    for (const auto& s : ensemble) total += static_cast<int>(s.instruments.size());
    EXPECT_EQ(total, 4);
}

TEST(OrchestrationEngineTest, OrchestrateChords) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("pop", 4);
    auto result = engine->orchestrate_chords({"C", "F", "G", "C"}, ensemble, 2, 120, 42);
    EXPECT_FALSE(result.empty());
    for (const auto& [name, events] : result) {
        EXPECT_FALSE(events.empty());
    }
}

TEST(OrchestrationEngineTest, OrchestrateFull) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("pop", 4);
    std::vector<MidiEvent> melody;
    melody.push_back({0, 240, 0, 60, 100, 0});   // C4
    melody.push_back({240, 240, 0, 62, 100, 0}); // D4
    melody.push_back({480, 240, 0, 64, 100, 0}); // E4
    melody.push_back({720, 240, 0, 60, 100, 0}); // C4
    auto result = engine->orchestrate_full(melody, {"C", "F", "G", "C"}, ensemble, 2, 120, 42);
    EXPECT_FALSE(result.empty());
}

TEST(OrchestrationEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("pop", 4);
    auto a = engine->orchestrate_chords({"C", "F", "G", "C"}, ensemble, 2, 120, 42);
    auto b = engine->orchestrate_chords({"C", "F", "G", "C"}, ensemble, 2, 120, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        ASSERT_EQ(a[i].second.size(), b[i].second.size());
        for (std::size_t j = 0; j < a[i].second.size(); ++j) {
            EXPECT_EQ(a[i].second[j].note, b[i].second[j].note);
            EXPECT_EQ(a[i].second[j].tick_on, b[i].second[j].tick_on);
        }
    }
}

TEST(OrchestrationEngineTest, RangeRespected) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("orchestra", 8);
    auto result = engine->orchestrate_chords({"C", "F", "G", "C"}, ensemble, 2, 120, 42);
    std::unordered_map<std::string, OrchestralInstrument> inst_map;
    for (const auto& section : ensemble) {
        for (const auto& inst : section.instruments) {
            inst_map[inst.name] = inst;
        }
    }
    for (const auto& [name, events] : result) {
        auto it = inst_map.find(name);
        if (it != inst_map.end()) {
            for (const auto& e : events) {
                EXPECT_GE(e.note, it->second.min_note);
                EXPECT_LE(e.note, it->second.max_note);
            }
        }
    }
}

TEST(OrchestrationEngineTest, JazzEnsembleHasFiveInstruments) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("jazz", 5);
    int total = 0;
    for (const auto& s : ensemble) total += static_cast<int>(s.instruments.size());
    EXPECT_EQ(total, 5);
}

TEST(OrchestrationEngineTest, RockEnsembleHasFourInstruments) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("rock", 0);
    int total = 0;
    for (const auto& s : ensemble) total += static_cast<int>(s.instruments.size());
    EXPECT_EQ(total, 4);
}

TEST(OrchestrationEngineTest, EmptyInputReturnsEmpty) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("pop", 4);
    auto result = engine->orchestrate_chords({}, ensemble, 2, 120, 42);
    EXPECT_TRUE(result.empty());
}

TEST(OrchestrationEngineTest, SingleChordWithQuartet) {
    auto engine = make_engine();
    auto ensemble = engine->suggest_ensemble("string quartet", 0);
    auto result = engine->orchestrate_chords({"C"}, ensemble, 1, 120, 42);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), 4u);
}
