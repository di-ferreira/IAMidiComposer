// Test: theory/harmony (skeleton).
//
// Currently asserts the DI wiring works: HarmonyEngine built with a ScaleProvider.
// Golden MIDI tests will be added once the actual chord rendering lands.
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/core/ServiceLocator.hpp>

#include <gtest/gtest.h>

using namespace aimidi::theory;

TEST(HarmonyEngineTest, EmptyBlueprintReturnsEmptyMidi) {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    ASSERT_TRUE(scales);
    auto engine = make_harmony_engine(scales);
    ASSERT_TRUE(engine);
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto events = engine->generate(req);
    EXPECT_TRUE(events.empty());  // skeleton: returns empty.
}

TEST(HarmonyEngineTest, ResolveableViaServiceLocator) {
    auto loc = aimidi::core::make_production();
    auto sp = loc.resolve<IScaleProvider>();
    ASSERT_TRUE(sp);
    EXPECT_TRUE(sp->knows("major"));
    auto he = loc.resolve<IHarmonyEngine>();
    ASSERT_TRUE(he);
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto events = he->generate(req);
    EXPECT_TRUE(events.empty());  // skeleton: returns empty.
}
