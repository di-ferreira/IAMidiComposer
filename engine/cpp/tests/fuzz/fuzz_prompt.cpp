// Fuzz target for prompt input validation
#include <cstdint>
#include <cstddef>
#include <string>
#include <aimidi/theory/InputValidation.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);
    auto result = aimidi::theory::InputValidation::validate_prompt(input);
    // Should never crash
    (void)result;
    return 0;
}
