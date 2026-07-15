#pragma once
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <memory>

namespace aimidi::theory {

class IBlueprintGenerator {
public:
    virtual ~IBlueprintGenerator() = default;

    virtual MusicBlueprint generate(const MusicBlueprint& input) const = 0;
};

std::unique_ptr<IBlueprintGenerator> make_blueprint_generator();

} // namespace aimidi::theory
