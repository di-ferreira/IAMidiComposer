#pragma once
#include <aimidi/theory/Types.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

// Detected style information.
struct StyleInfo {
    std::string              genre;       // primary genre: "pop", "rock", "jazz", etc.
    std::vector<std::string> subgenres;   // secondary genres/tags
    float                    confidence;  // 0..1
    std::string              mood;        // "happy", "sad", "energetic", "calm", etc.
    float                    tempo_guess; // estimated BPM
    std::string              key_guess;   // estimated key
};

// StyleDetection: analyzes text or audio features to determine musical style.
class IStyleDetection {
public:
    virtual ~IStyleDetection() = default;

    // Analyze a text prompt for style information.
    virtual StyleInfo analyze_text(std::string_view prompt, Seed seed = 0) const = 0;

    // Analyze audio features (float vector) for style information.
    // The features vector format depends on the implementation.
    // For the stub, this is a no-op.
    virtual StyleInfo analyze_audio(const std::vector<float>& features, Seed seed = 0) const = 0;
};

// Factory: returns a stub StyleDetection.
std::unique_ptr<IStyleDetection> make_style_detection();

} // namespace aimidi::theory
