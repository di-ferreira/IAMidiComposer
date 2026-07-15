#include <aimidi/theory/IStyleDetection.hpp>
#include <algorithm>
#include <cctype>

namespace aimidi::theory {

namespace {

std::string to_lower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

class StubStyleDetection final : public IStyleDetection {
public:
    StyleInfo analyze_text(std::string_view prompt, Seed seed) const override {
        (void)seed;
        StyleInfo info{};
        info.genre = "pop";
        info.subgenres = {"pop", "catchy"};
        info.confidence = 0.5f;
        info.mood = "neutral";
        info.tempo_guess = 120.0f;
        info.key_guess = "C";

        std::string lower = to_lower(prompt);

        // Simple keyword matching
        struct GenreMatch { std::string_view keyword; std::string_view genre; std::string_view mood; float tempo; };
        const GenreMatch matches[] = {
            {"rock",     "rock",     "energetic", 130.0f},
            {"jazz",     "jazz",     "sophisticated", 110.0f},
            {"blues",    "blues",    "melancholic", 100.0f},
            {"electronic", "electronic", "energetic", 128.0f},
            {"edm",      "electronic", "energetic", 128.0f},
            {"techno",   "electronic", "intense", 140.0f},
            {"hip hop",  "hip_hop",  "confident", 95.0f},
            {"rap",      "hip_hop",  "confident", 95.0f},
            {"rnb",      "rnb",      "smooth", 90.0f},
            {"funk",     "funk",     "groovy", 110.0f},
            {"soul",     "soul",     "emotional", 100.0f},
            {"classical","classical","serene", 80.0f},
            {"latin",    "latin",    "passionate", 120.0f},
            {"bossa",    "latin",    "relaxed", 120.0f},
            {"country",  "country",  "nostalgic", 100.0f},
            {"metal",    "metal",    "aggressive", 160.0f},
            {"folk",     "folk",     "earthy", 100.0f},
        };

        for (const auto& m : matches) {
            if (lower.find(m.keyword) != std::string::npos) {
                info.genre = std::string(m.genre);
                info.mood = std::string(m.mood);
                info.tempo_guess = m.tempo;
                info.confidence = 0.7f;
                break;
            }
        }

        return info;
    }

    StyleInfo analyze_audio(const std::vector<float>& features, Seed seed) const override {
        (void)features;
        (void)seed;
        StyleInfo info{};
        info.genre = "pop";
        info.subgenres = {"pop"};
        info.confidence = 0.3f;  // low confidence without real audio analysis
        info.mood = "neutral";
        info.tempo_guess = 120.0f;
        info.key_guess = "C";
        return info;
    }
};

} // anonymous namespace

std::unique_ptr<IStyleDetection> make_style_detection() {
    return std::make_unique<StubStyleDetection>();
}

} // namespace aimidi::theory
