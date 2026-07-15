#include <aimidi/theory/IInstrumentMapper.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace aimidi::theory {

namespace {

// ---------------------------------------------------------------------------
//  GM Program table (General MIDI Level 1 — programs 1..128, converted to 0..127)
// ---------------------------------------------------------------------------

struct GmEntry {
    int         program;
    std::string_view name;
    std::string_view family;
};

constexpr std::array kGmPrograms = {
    // Piano family (1-8)
    GmEntry{  1, "Acoustic Grand Piano",      "Piano"},
    GmEntry{  2, "Bright Acoustic Piano",     "Piano"},
    GmEntry{  3, "Electric Grand Piano",      "Piano"},
    GmEntry{  4, "Honky-tonk Piano",          "Piano"},
    GmEntry{  5, "Electric Piano 1",          "Piano"},
    GmEntry{  6, "Electric Piano 2",          "Piano"},
    GmEntry{  7, "Harpsichord",               "Piano"},
    GmEntry{  8, "Clavinet",                  "Piano"},
    // Chromatic Percussion (9-16)
    GmEntry{  9, "Celesta",                   "Chromatic Percussion"},
    GmEntry{ 10, "Glockenspiel",              "Chromatic Percussion"},
    GmEntry{ 11, "Music Box",                 "Chromatic Percussion"},
    GmEntry{ 12, "Vibraphone",                "Chromatic Percussion"},
    GmEntry{ 13, "Marimba",                   "Chromatic Percussion"},
    GmEntry{ 14, "Xylophone",                 "Chromatic Percussion"},
    GmEntry{ 15, "Tubular Bells",             "Chromatic Percussion"},
    GmEntry{ 16, "Dulcimer",                  "Chromatic Percussion"},
    // Organ (17-24)
    GmEntry{ 17, "Drawbar Organ",             "Organ"},
    GmEntry{ 18, "Percussive Organ",          "Organ"},
    GmEntry{ 19, "Rock Organ",                "Organ"},
    GmEntry{ 20, "Church Organ",              "Organ"},
    GmEntry{ 21, "Reed Organ",                "Organ"},
    GmEntry{ 22, "Accordion",                 "Organ"},
    GmEntry{ 23, "Harmonica",                 "Organ"},
    GmEntry{ 24, "Tango Accordion",           "Organ"},
    // Guitar (25-32)
    GmEntry{ 25, "Acoustic Guitar (nylon)",   "Guitar"},
    GmEntry{ 26, "Acoustic Guitar (steel)",   "Guitar"},
    GmEntry{ 27, "Electric Guitar (jazz)",    "Guitar"},
    GmEntry{ 28, "Electric Guitar (clean)",   "Guitar"},
    GmEntry{ 29, "Electric Guitar (muted)",   "Guitar"},
    GmEntry{ 30, "Overdriven Guitar",         "Guitar"},
    GmEntry{ 31, "Distortion Guitar",         "Guitar"},
    GmEntry{ 32, "Guitar Harmonics",          "Guitar"},
    // Bass (33-40)
    GmEntry{ 33, "Acoustic Bass",             "Bass"},
    GmEntry{ 34, "Electric Bass (finger)",    "Bass"},
    GmEntry{ 35, "Electric Bass (pick)",      "Bass"},
    GmEntry{ 36, "Fretless Bass",             "Bass"},
    GmEntry{ 37, "Slap Bass 1",               "Bass"},
    GmEntry{ 38, "Slap Bass 2",               "Bass"},
    GmEntry{ 39, "Synth Bass 1",              "Bass"},
    GmEntry{ 40, "Synth Bass 2",              "Bass"},
    // Strings (41-48)
    GmEntry{ 41, "Violin",                    "Strings"},
    GmEntry{ 42, "Viola",                     "Strings"},
    GmEntry{ 43, "Cello",                     "Strings"},
    GmEntry{ 44, "Contrabass",                "Strings"},
    GmEntry{ 45, "Tremolo Strings",           "Strings"},
    GmEntry{ 46, "Pizzicato Strings",         "Strings"},
    GmEntry{ 47, "Orchestral Harp",           "Strings"},
    GmEntry{ 48, "Timpani",                   "Strings"},
    // Ensemble (49-56)
    GmEntry{ 49, "String Ensemble 1",         "Ensemble"},
    GmEntry{ 50, "String Ensemble 2",         "Ensemble"},
    GmEntry{ 51, "Synth Strings 1",           "Ensemble"},
    GmEntry{ 52, "Synth Strings 2",           "Ensemble"},
    GmEntry{ 53, "Choir Aahs",                "Ensemble"},
    GmEntry{ 54, "Voice Oohs",                "Ensemble"},
    GmEntry{ 55, "Synth Voice",               "Ensemble"},
    GmEntry{ 56, "Orchestra Hit",             "Ensemble"},
    // Brass (57-64)
    GmEntry{ 57, "Trumpet",                   "Brass"},
    GmEntry{ 58, "Trombone",                  "Brass"},
    GmEntry{ 59, "Tuba",                      "Brass"},
    GmEntry{ 60, "Muted Trumpet",             "Brass"},
    GmEntry{ 61, "French Horn",               "Brass"},
    GmEntry{ 62, "Brass Section",             "Brass"},
    GmEntry{ 63, "Synth Brass 1",             "Brass"},
    GmEntry{ 64, "Synth Brass 2",             "Brass"},
    // Reed (65-72)
    GmEntry{ 65, "Soprano Sax",               "Reed"},
    GmEntry{ 66, "Alto Sax",                  "Reed"},
    GmEntry{ 67, "Tenor Sax",                 "Reed"},
    GmEntry{ 68, "Baritone Sax",              "Reed"},
    GmEntry{ 69, "Oboe",                      "Reed"},
    GmEntry{ 70, "English Horn",              "Reed"},
    GmEntry{ 71, "Bassoon",                   "Reed"},
    GmEntry{ 72, "Clarinet",                  "Reed"},
    // Pipe (73-80)
    GmEntry{ 73, "Piccolo",                   "Pipe"},
    GmEntry{ 74, "Flute",                     "Pipe"},
    GmEntry{ 75, "Recorder",                  "Pipe"},
    GmEntry{ 76, "Pan Flute",                 "Pipe"},
    GmEntry{ 77, "Blown Bottle",              "Pipe"},
    GmEntry{ 78, "Shakuhachi",                "Pipe"},
    GmEntry{ 79, "Whistle",                   "Pipe"},
    GmEntry{ 80, "Ocarina",                   "Pipe"},
    // Synth Lead (81-88)
    GmEntry{ 81, "Lead 1 (square)",           "Synth Lead"},
    GmEntry{ 82, "Lead 2 (sawtooth)",         "Synth Lead"},
    GmEntry{ 83, "Lead 3 (calliope)",         "Synth Lead"},
    GmEntry{ 84, "Lead 4 (chiff)",            "Synth Lead"},
    GmEntry{ 85, "Lead 5 (charang)",          "Synth Lead"},
    GmEntry{ 86, "Lead 6 (voice)",            "Synth Lead"},
    GmEntry{ 87, "Lead 7 (fifths)",           "Synth Lead"},
    GmEntry{ 88, "Lead 8 (bass + lead)",      "Synth Lead"},
    // Synth Pad (89-96)
    GmEntry{ 89, "Pad 1 (new age)",           "Synth Pad"},
    GmEntry{ 90, "Pad 2 (warm)",              "Synth Pad"},
    GmEntry{ 91, "Pad 3 (polysynth)",         "Synth Pad"},
    GmEntry{ 92, "Pad 4 (choir)",             "Synth Pad"},
    GmEntry{ 93, "Pad 5 (bowed)",             "Synth Pad"},
    GmEntry{ 94, "Pad 6 (metallic)",          "Synth Pad"},
    GmEntry{ 95, "Pad 7 (halo)",              "Synth Pad"},
    GmEntry{ 96, "Pad 8 (sweep)",             "Synth Pad"},
    // Synth Effects (97-104)
    GmEntry{ 97, "FX 1 (rain)",               "Synth FX"},
    GmEntry{ 98, "FX 2 (soundtrack)",         "Synth FX"},
    GmEntry{ 99, "FX 3 (crystal)",            "Synth FX"},
    GmEntry{100, "FX 4 (atmosphere)",         "Synth FX"},
    GmEntry{101, "FX 5 (brightness)",         "Synth FX"},
    GmEntry{102, "FX 6 (goblins)",            "Synth FX"},
    GmEntry{103, "FX 7 (echoes)",             "Synth FX"},
    GmEntry{104, "FX 8 (sci-fi)",             "Synth FX"},
    // Ethnic (105-112)
    GmEntry{105, "Sitar",                     "Ethnic"},
    GmEntry{106, "Banjo",                     "Ethnic"},
    GmEntry{107, "Shamisen",                  "Ethnic"},
    GmEntry{108, "Koto",                      "Ethnic"},
    GmEntry{109, "Kalimba",                   "Ethnic"},
    GmEntry{110, "Bagpipe",                   "Ethnic"},
    GmEntry{111, "Fiddle",                    "Ethnic"},
    GmEntry{112, "Shanai",                    "Ethnic"},
    // Percussive (113-120)
    GmEntry{113, "Tinkle Bell",               "Percussive"},
    GmEntry{114, "Agogo",                     "Percussive"},
    GmEntry{115, "Steel Drums",               "Percussive"},
    GmEntry{116, "Woodblock",                 "Percussive"},
    GmEntry{117, "Taiko Drum",                "Percussive"},
    GmEntry{118, "Melodic Tom",               "Percussive"},
    GmEntry{119, "Synth Drum",                "Percussive"},
    GmEntry{120, "Reverse Cymbal",            "Percussive"},
    // Sound Effects (121-128)
    GmEntry{121, "Guitar Fret Noise",         "Sound Effects"},
    GmEntry{122, "Breath Noise",              "Sound Effects"},
    GmEntry{123, "Seashore",                  "Sound Effects"},
    GmEntry{124, "Bird Tweet",                "Sound Effects"},
    GmEntry{125, "Telephone Ring",            "Sound Effects"},
    GmEntry{126, "Helicopter",                "Sound Effects"},
    GmEntry{127, "Applause",                  "Sound Effects"},
    GmEntry{128, "Gunshot",                   "Sound Effects"},
};

// ---------------------------------------------------------------------------
//  Name-based lookup helpers
// ---------------------------------------------------------------------------

[[nodiscard]] std::string to_lower(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    for (char c : s)
        r.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return r;
}

[[nodiscard]] bool contains(std::string_view haystack, std::string_view needle) {
    return haystack.find(needle) != std::string_view::npos;
}

// ---------------------------------------------------------------------------
//  InstrumentMapper implementation
// ---------------------------------------------------------------------------

class InstrumentMapper final : public IInstrumentMapper {
public:
    GmProgram map_to_gm(std::string_view name, std::string_view role) const override {
        auto lower_name = to_lower(name);
        auto lower_role = to_lower(role);

        // Role-based match first (more specific).
        if (contains(lower_role, "drum") || contains(lower_name, "drum"))
            return {1, "Standard Kit", "Drums"};

        if (contains(lower_name, "piano") || contains(lower_name, "keys") ||
            contains(lower_role, "piano"))
            return find_or_default("Piano", 1);

        if (contains(lower_name, "bass"))
            return find_or_default("Bass", 34);

        if (contains(lower_name, "guitar"))
            return find_or_default("Guitar", 27);

        if (contains(lower_name, "string"))
            return find_or_default("Ensemble", 49);

        if (contains(lower_name, "synth"))
            return find_or_default("Synth Lead", 81);

        if (contains(lower_name, "brass"))
            return find_or_default("Brass", 62);

        if (contains(lower_name, "sax"))
            return find_or_default("Reed", 66);

        if (contains(lower_name, "horn"))
            return find_or_default("Brass", 61);

        if (contains(lower_name, "organ"))
            return find_or_default("Organ", 17);

        if (contains(lower_name, "violin") || contains(lower_name, "viola") ||
            contains(lower_name, "cello"))
            return find_or_default("Strings", 41);

        if (contains(lower_name, "flute") || contains(lower_name, "piccolo"))
            return find_or_default("Pipe", 74);

        if (contains(lower_name, "trumpet"))
            return find_or_default("Brass", 57);

        if (contains(lower_name, "trombone"))
            return find_or_default("Brass", 58);

        return find_or_default("Piano", 1);
    }

    InstrumentMapping replace_instrument(
        const BlueprintInstrument& source,
        std::string_view target_name,
        std::string_view target_role) const override
    {
        InstrumentMapping result;
        result.source = source;

        auto target_gm = map_to_gm(target_name, target_role);

        result.target.name    = std::string(target_name);
        result.target.role    = std::string(target_role);
        result.target.channel = source.channel;
        result.target.program = target_gm.program;
        result.success        = true;

        return result;
    }

    std::vector<GmProgram> programs_for_family(std::string_view family) const override {
        std::vector<GmProgram> out;
        for (auto& e : kGmPrograms) {
            if (e.family == family)
                out.push_back({e.program, std::string(e.name), std::string(e.family)});
        }
        return out;
    }

    std::vector<std::string> families() const override {
        std::vector<std::string> out;
        for (auto& e : kGmPrograms) {
            if (out.empty() || out.back() != e.family)
                out.push_back(std::string(e.family));
        }
        return out;
    }

private:
    [[nodiscard]] GmProgram find_or_default(std::string_view /*family*/, int default_prog) const {
        for (auto& e : kGmPrograms) {
            if (e.program == default_prog)
                return {e.program, std::string(e.name), std::string(e.family)};
        }
        return {1, "Acoustic Grand Piano", "Piano"};
    }
};

} // anonymous namespace

std::unique_ptr<IInstrumentMapper> make_instrument_mapper() {
    return std::make_unique<InstrumentMapper>();
}

} // namespace aimidi::theory
