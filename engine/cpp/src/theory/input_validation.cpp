#include <aimidi/theory/InputValidation.hpp>
#include <algorithm>
#include <cctype>
#include <string>

namespace aimidi::theory {

const std::vector<std::string_view>& InputValidation::forbidden_patterns() {
    static const std::vector<std::string_view> patterns = {
        // Direct note requests
        "note c", "note d", "note e", "note f", "note g", "note a", "note b",
        "notes c", "notes d", "notes e", "notes f", "notes g", "notes a", "notes b",
        "midi note", "midi notes",
        "write note", "write notes",
        "generate note", "generate notes",
        "play note", "play notes",
        "create note", "create notes",
        // Specific note patterns
        "c4 ", "d4 ", "e4 ", "f4 ", "g4 ", "a4 ", "b4 ",
        " c4", " d4", " e4", " f4", " g4", " a4", " b4",
        // Explicit MIDI
        "midi number", "midi value",
        "note number", "note value",
        "pitch 6", "pitch 7", "pitch 8", "pitch 9",
        // Velocity requests
        "velocity 12", "velocity 6", "velocity 8",
        // Explicit chord spelling
        "c e g", "c e g ", "c e g b", "d f a", "e g b",
        "c major chord notes", "c minor chord notes",
    };
    return patterns;
}

std::string to_lower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

bool InputValidation::contains_forbidden_patterns(std::string_view prompt) {
    std::string lower = to_lower(prompt);

    for (auto pattern : forbidden_patterns()) {
        if (lower.find(pattern) != std::string::npos) {
            return true;
        }
    }

    return false;
}

std::string InputValidation::sanitize(std::string_view prompt) {
    // Truncate to max length
    std::string result(prompt.substr(0, kMaxPromptLength));

    // Remove any obvious note patterns (basic sanitization)
    // This is a simple implementation - a real one would be more sophisticated
    return result;
}

ValidationResult InputValidation::validate_prompt(std::string_view prompt) {
    ValidationResult result{};

    // Check empty
    if (prompt.empty()) {
        result.valid = false;
        result.error = "Prompt cannot be empty";
        return result;
    }

    // Check max length
    if (prompt.size() > kMaxPromptLength) {
        result.valid = false;
        result.error = "Prompt exceeds maximum length of " + std::to_string(kMaxPromptLength) + " characters";
        return result;
    }

    // Check forbidden patterns
    if (contains_forbidden_patterns(prompt)) {
        result.valid = false;
        result.error = "Prompt contains forbidden patterns: asking for literal MIDI notes is not supported. "
                       "Describe the musical style, mood, and structure instead.";
        return result;
    }

    // Sanitize
    result.sanitized = sanitize(prompt);
    result.valid = true;
    return result;
}

} // namespace aimidi::theory
