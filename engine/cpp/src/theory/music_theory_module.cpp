// Music Theory Engine — module registration for the ServiceLocator (ADR-0002).
//
// Idempotent, topologically ordered: IScaleProvider first (no deps),
// IHarmonyEngine second (depends on IScaleProvider resolved lazily from the
// same locator).
#include <aimidi/core/ServiceLocator.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>

#include <memory>

namespace aimidi::core {

void register_music_theory(ServiceLocator& loc) {
    if (!loc.has<aimidi::theory::IScaleProvider>()) {
        loc.bind<aimidi::theory::IScaleProvider>(
            []() -> std::shared_ptr<aimidi::theory::IScaleProvider> {
                return std::shared_ptr<aimidi::theory::IScaleProvider>(
                    aimidi::theory::make_scale_provider().release());
            });
    }

    if (!loc.has<aimidi::theory::IHarmonyEngine>()) {
        // Capture a pointer to the locator: the locator must outlive the
        // resolved HarmonyEngine instance (lifetime contract documented in
        // ServiceLocator.hpp). Lazy resolution happens at resolve() time so
        // that a mock IScaleProvider registered after register_music_theory
        // would still be picked up.
        const ServiceLocator* loc_ptr = &loc;
        loc.bind<aimidi::theory::IHarmonyEngine>(
            [loc_ptr]() -> std::shared_ptr<aimidi::theory::IHarmonyEngine> {
                auto scales = loc_ptr->resolve<aimidi::theory::IScaleProvider>();
                return std::shared_ptr<aimidi::theory::IHarmonyEngine>(
                    aimidi::theory::make_harmony_engine(std::move(scales)).release());
            });
    }
}

ServiceLocator make_production() {
    ServiceLocator loc;
    register_music_theory(loc);
    return loc;
}

ServiceLocator make_empty() {
    return ServiceLocator{};
}

} // namespace aimidi::core
