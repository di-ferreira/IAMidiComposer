// Music Theory Engine — module registration for the ServiceLocator (ADR-0002).
//
// Idempotent, topologically ordered: IScaleProvider first (no deps),
// IChordEngine second (no deps), IHarmonyEngine last (depends on both,
// resolved lazily from the same locator).
#include <aimidi/core/ServiceLocator.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/IRhythmEngine.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IDrumEngine.hpp>
#include <aimidi/theory/IPianoEngine.hpp>
#include <aimidi/theory/IHumanizationEngine.hpp>
#include <aimidi/theory/IMidiRenderer.hpp>

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

    if (!loc.has<aimidi::theory::IChordEngine>()) {
        loc.bind<aimidi::theory::IChordEngine>(
            []() -> std::shared_ptr<aimidi::theory::IChordEngine> {
                return std::shared_ptr<aimidi::theory::IChordEngine>(
                    aimidi::theory::make_chord_engine().release());
            });
    }

    if (!loc.has<aimidi::theory::IBassEngine>()) {
        loc.bind<aimidi::theory::IBassEngine>(
            []() -> std::shared_ptr<aimidi::theory::IBassEngine> {
                return std::shared_ptr<aimidi::theory::IBassEngine>(
                    aimidi::theory::make_bass_engine().release());
            });
    }

    if (!loc.has<aimidi::theory::IHarmonyEngine>()) {
        const ServiceLocator* loc_ptr = &loc;
        loc.bind<aimidi::theory::IHarmonyEngine>(
            [loc_ptr]() -> std::shared_ptr<aimidi::theory::IHarmonyEngine> {
                auto scales = loc_ptr->resolve<aimidi::theory::IScaleProvider>();
                std::shared_ptr<aimidi::theory::IChordEngine> chords;
                if (loc_ptr->has<aimidi::theory::IChordEngine>()) {
                    chords = loc_ptr->resolve<aimidi::theory::IChordEngine>();
                }
                return std::shared_ptr<aimidi::theory::IHarmonyEngine>(
                    aimidi::theory::make_harmony_engine(std::move(scales),
                                                        std::move(chords)).release());
            });
    }

    if (!loc.has<aimidi::theory::IDrumEngine>()) {
        loc.bind<aimidi::theory::IDrumEngine>(
            []() -> std::shared_ptr<aimidi::theory::IDrumEngine> {
                return std::shared_ptr<aimidi::theory::IDrumEngine>(
                    aimidi::theory::make_drum_engine().release());
            });
    }

    if (!loc.has<aimidi::theory::IPianoEngine>()) {
        loc.bind<aimidi::theory::IPianoEngine>(
            []() -> std::shared_ptr<aimidi::theory::IPianoEngine> {
                return std::shared_ptr<aimidi::theory::IPianoEngine>(
                    aimidi::theory::make_piano_engine().release());
            });
    }

    if (!loc.has<aimidi::theory::IHumanizationEngine>()) {
        loc.bind<aimidi::theory::IHumanizationEngine>(
            []() -> std::shared_ptr<aimidi::theory::IHumanizationEngine> {
                return std::shared_ptr<aimidi::theory::IHumanizationEngine>(
                    aimidi::theory::make_humanization_engine().release());
            });
    }

    if (!loc.has<aimidi::theory::IMidiRenderer>()) {
        loc.bind<aimidi::theory::IMidiRenderer>(
            []() -> std::shared_ptr<aimidi::theory::IMidiRenderer> {
                return std::shared_ptr<aimidi::theory::IMidiRenderer>(
                    aimidi::theory::make_midi_renderer().release());
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
