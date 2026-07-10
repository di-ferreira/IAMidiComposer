// ServiceLocator — minimal DI container for the Music Theory Engine (MTE).
//
// ADR-0002: a small, explicit, header-only ServiceLocator that stores lazy
// factories keyed by std::type_index. No auto-wiring: each `register_*` module
// function is responsible for topological ordering of its own dependencies.
//
// Lifetime contract:
//   The ServiceLocator instance MUST outlive every shared_ptr resolved from it,
//   because factory lambdas may capture a reference to the locator in order to
//   lazy-resolve their own dependencies (see register_music_theory).
//
// Thread-safety: const operations (resolve/has) are safe for concurrent reads
// after registration is complete. Mutating operations (bind/clear) are NOT
// thread-safe and must happen during setup only.
#pragma once

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <string>

namespace aimidi::core {

class ServiceLocator {
public:
    ServiceLocator() = default;
    ~ServiceLocator() = default;

    ServiceLocator(const ServiceLocator&) = default;
    ServiceLocator(ServiceLocator&&) noexcept = default;
    ServiceLocator& operator=(const ServiceLocator&) = default;
    ServiceLocator& operator=(ServiceLocator&&) noexcept = default;

    /// Register a lazy factory for interface I. Idempotent from the caller's
    /// perspective: calling bind twice overwrites the previous binding.
    template <class I>
    void bind(std::function<std::shared_ptr<I>()> factory) {
        factories_[std::type_index(typeid(I))] =
            [f = std::move(factory)]() -> std::shared_ptr<void> {
            return std::shared_ptr<void>(
                std::static_pointer_cast<void>(f()));
        };
    }

    /// Resolve interface I. Throws std::runtime_error if not registered.
    template <class I>
    [[nodiscard]] std::shared_ptr<I> resolve() const {
        const auto key = std::type_index(typeid(I));
        const auto it = factories_.find(key);
        if (it == factories_.end()) {
            throw std::runtime_error(
                "aimidi::core::ServiceLocator: unregistered type: " +
                std::string(typeid(I).name()));
        }
        auto erased = it->second();
        return std::static_pointer_cast<I>(erased);
    }

    /// True if interface I has a registered factory.
    template <class I>
    [[nodiscard]] bool has() const noexcept {
        return factories_.find(std::type_index(typeid(I))) != factories_.end();
    }

    /// Remove all bindings. Useful for test isolation.
    void clear() noexcept {
        factories_.clear();
    }

private:
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
};

/// Register the music-theory module bindings (IScaleProvider, IHarmonyEngine)
/// into the given ServiceLocator. Idempotent: calling twice is a no-op.
/// Defined in music_theory_module.cpp.
void register_music_theory(ServiceLocator& loc);

/// Build a ServiceLocator with all production music-theory bindings registered.
/// Convenience for production code and integration tests.
[[nodiscard]] ServiceLocator make_production();

/// Build an empty ServiceLocator with no bindings. Convenience for tests that
/// want full control over which implementations/mocks to register.
[[nodiscard]] ServiceLocator make_empty();

} // namespace aimidi::core
