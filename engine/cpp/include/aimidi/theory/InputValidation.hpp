#pragma once
#include <string_view>
#include <string>
#include <vector>

namespace aimidi::theory {

// Result of input validation.
struct ValidationResult {
    bool   valid = true;
    std::string error;       // empty if valid
    std::string sanitized;   // sanitized version of input (may be empty on rejection)
};

// Input validation for user prompts.
// Ensures the user is not asking the AI to generate literal MIDI notes,
// which would violate the project's core principle.
class InputValidation {
public:
    // Validate a user prompt.
    // Returns a ValidationResult with:
    // - valid=false + error message if the prompt is rejected
    // - valid=true + sanitized prompt if accepted
    static ValidationResult validate_prompt(std::string_view prompt);

    // Check if a prompt contains forbidden patterns (literal note requests).
    static bool contains_forbidden_patterns(std::string_view prompt);

    // Sanitize a prompt by removing or replacing problematic patterns.
    static std::string sanitize(std::string_view prompt);

private:
    // Forbidden patterns that ask for literal MIDI notes.
    static const std::vector<std::string_view>& forbidden_patterns();

    // Maximum prompt length.
    static constexpr std::size_t kMaxPromptLength = 4096;
};

} // namespace aimidi::theory
