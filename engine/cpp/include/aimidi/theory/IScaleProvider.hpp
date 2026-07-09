// Scale provider interface.
//
// Implementations must be deterministic and O(1) for `intervals_of()`.
// Concrete provider is injected via DI (Music Theory Engine factory).
#pragma once

#include <aimidi/theory/Types.hpp>
#include <span>
#include <string_view>
#include <memory>

namespace aimidi::theory {

class IScaleProvider {
public:
    virtual ~IScaleProvider() = default;

    /// Intervals (in semitons) of a named scale, e.g. "major" => {0,2,4,5,7,9,11}.
    virtual std::span<const int> intervals_of(std::string_view name) const = 0;

    /// True if scale name is known (resolved by implementation).
    virtual bool knows(std::string_view name) const noexcept = 0;
};

/// Factory: returns a default ScaleProvider with the standard scales wired in.
/// Provided so the engine (and tests) can use DI without depending on the
/// concrete class declaration.
std::unique_ptr<IScaleProvider> make_scale_provider();

} // namespace aimidi::theory
