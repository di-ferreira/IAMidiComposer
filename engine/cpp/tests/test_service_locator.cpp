// Test: core/ServiceLocator — ADR-0002 DI container.
//
// Exercises: bind/resolve, throw on unregistered, music-theory module wiring,
// idempotent registration. Uses GMock to prove mocks work with the container.
#include <aimidi/core/ServiceLocator.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <stdexcept>
#include <string>

// ─── Mock interface for generic ServiceLocator behaviour ───────────────
namespace {

class IMockService {
public:
    virtual ~IMockService() = default;
    virtual int value() const = 0;
};

class MockService : public IMockService {
public:
    MOCK_METHOD(int, value, (), (const, override));
};

// A fake interface used only to test the "unregistered" error path.
class IMissingService {
public:
    virtual ~IMissingService() = default;
    virtual void noop() = 0;
};

} // namespace

// ─── Tests ──────────────────────────────────────────────────────────────
using aimidi::core::ServiceLocator;
using aimidi::core::make_production;
using aimidi::core::make_empty;
using aimidi::core::register_music_theory;

TEST(ServiceLocatorTest, ResolvesRegistered) {
    auto loc = make_empty();
    loc.bind<IMockService>([]() -> std::shared_ptr<IMockService> {
        return std::make_shared<::testing::StrictMock<MockService>>();
    });
    auto svc = loc.resolve<IMockService>();
    ASSERT_TRUE(svc);
    EXPECT_CALL(*dynamic_cast<MockService*>(svc.get()), value())
        .WillOnce(::testing::Return(42));
    EXPECT_EQ(svc->value(), 42);
}

TEST(ServiceLocatorTest, ThrowsOnUnregistered) {
    auto loc = make_empty();
    try {
        (void)loc.resolve<IMissingService>();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& ex) {
        const std::string msg = ex.what();
        EXPECT_NE(msg.find("unregistered"), std::string::npos)
            << "Error message should contain 'unregistered', got: " << msg;
    } catch (...) {
        FAIL() << "Expected std::runtime_error, got different exception";
    }
}

TEST(ServiceLocatorTest, HasReportsRegistration) {
    auto loc = make_empty();
    EXPECT_FALSE(loc.has<IMockService>());
    loc.bind<IMockService>([]() -> std::shared_ptr<IMockService> {
        return std::make_shared<::testing::NiceMock<MockService>>();
    });
    EXPECT_TRUE(loc.has<IMockService>());
}

TEST(ServiceLocatorTest, ClearRemovesAllBindings) {
    auto loc = make_empty();
    loc.bind<IMockService>([]() -> std::shared_ptr<IMockService> {
        return std::make_shared<::testing::NiceMock<MockService>>();
    });
    EXPECT_TRUE(loc.has<IMockService>());
    loc.clear();
    EXPECT_FALSE(loc.has<IMockService>());
}

TEST(ServiceLocatorTest, MusicTheoryModule_RegistersScaleProvider) {
    auto loc = make_production();
    auto sp = loc.resolve<aimidi::theory::IScaleProvider>();
    ASSERT_TRUE(sp);
    EXPECT_TRUE(sp->knows("major"));
    EXPECT_TRUE(sp->knows("minor"));
    EXPECT_FALSE(sp->knows("does_not_exist"));
}

TEST(ServiceLocatorTest, MusicTheoryModule_RegistersHarmonyEngine) {
    auto loc = make_production();
    auto he = loc.resolve<aimidi::theory::IHarmonyEngine>();
    ASSERT_TRUE(he);
    const aimidi::theory::HarmonyRequest req{
        .root_key = "C",
        .scale = "major",
        .bars = 4,
        .seed = 42};
    const auto events = he->generate(req);
    EXPECT_FALSE(events.empty());  // harmony engine emits real MIDI now.
}

TEST(ServiceLocatorTest, IdempotentRegister) {
    auto loc = make_empty();
    register_music_theory(loc);
    register_music_theory(loc);
    EXPECT_TRUE(loc.has<aimidi::theory::IScaleProvider>());
    EXPECT_TRUE(loc.has<aimidi::theory::IHarmonyEngine>());
    auto sp = loc.resolve<aimidi::theory::IScaleProvider>();
    ASSERT_TRUE(sp);
    EXPECT_TRUE(sp->knows("major"));
}

TEST(ServiceLocatorTest, HarmonyResolvesScaleProviderFromLocator) {
    // Verify that IHarmonyEngine, when resolved, gets a working IScaleProvider.
    auto loc = make_production();
    auto he = loc.resolve<aimidi::theory::IHarmonyEngine>();
    ASSERT_TRUE(he);
    auto sp = loc.resolve<aimidi::theory::IScaleProvider>();
    ASSERT_TRUE(sp);
    EXPECT_EQ(sp->intervals_of("major").size(), 7u);
    const aimidi::theory::HarmonyRequest req{
        .root_key = "C",
        .scale = "minor",
        .bars = 2,
        .seed = 1};
    EXPECT_FALSE(he->generate(req).empty());
}
