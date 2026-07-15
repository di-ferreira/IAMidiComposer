#pragma once
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <aimidi/theory/Types.hpp>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace aimidi::theory {

// GM Program information.
struct GmProgram {
    int         program;
    std::string name;
    std::string family;
};

// Instrument mapping result.
struct InstrumentMapping {
    BlueprintInstrument source;
    BlueprintInstrument target;
    bool                success = false;
    std::string         warning;
};

// InstrumentMapper: maps instruments to GM programs and handles instrument replacement.
class IInstrumentMapper {
public:
    virtual ~IInstrumentMapper() = default;

    // Map an instrument name/role to a GM program.
    virtual GmProgram map_to_gm(std::string_view name, std::string_view role) const = 0;

    // Replace an instrument in an arrangement.
    // Returns the new instrument with updated GM program and channel.
    virtual InstrumentMapping replace_instrument(
        const BlueprintInstrument& source,
        std::string_view target_name,
        std::string_view target_role) const = 0;

    // Get all available GM programs for a family.
    virtual std::vector<GmProgram> programs_for_family(std::string_view family) const = 0;

    // Get all instrument families.
    virtual std::vector<std::string> families() const = 0;
};

std::unique_ptr<IInstrumentMapper> make_instrument_mapper();

} // namespace aimidi::theory
