#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace aimidi::theory {

// GM program category (blocks of 8 programs each, per GM1/2 standard).
enum class GMProgramCategory {
    piano,
    chromatic_percussion,
    organ,
    guitar,
    bass,
    strings,
    ensemble,
    brass,
    reed,
    pipe,
    synth_lead,
    synth_pad,
    synth_effects,
    ethnic,
    percussion,
    sound_effects
};

// GM program entry.
struct GMProgram {
    int             program;   // 0-127
    std::string_view name;
    GMProgramCategory category;
    int             min_note;  // typical playable range low
    int             max_note;  // typical playable range high
    bool            is_drum;   // true if a drum kit (channel 10)
};

// ── Internal tables ────────────────────────────────────────────────

namespace detail {

inline constexpr std::array kCategoryStarts = {
    0,    // piano
    8,    // chromatic_percussion
    16,   // organ
    24,   // guitar
    32,   // bass
    40,   // strings
    48,   // ensemble
    56,   // brass
    64,   // reed
    72,   // pipe
    80,   // synth_lead
    88,   // synth_pad
    96,   // synth_effects
    104,  // ethnic
    112,  // percussion
    120,  // sound_effects
};

inline constexpr int kProgramsPerCategory = 8;

inline constexpr std::array kGmPrograms = {
    // 0-7  Piano
    GMProgram{  0, "Acoustic Grand Piano",      GMProgramCategory::piano,               21, 108, false },
    GMProgram{  1, "Bright Acoustic Piano",     GMProgramCategory::piano,               21, 108, false },
    GMProgram{  2, "Electric Grand Piano",       GMProgramCategory::piano,               21, 108, false },
    GMProgram{  3, "Honky-tonk Piano",           GMProgramCategory::piano,               21, 108, false },
    GMProgram{  4, "Electric Piano 1",           GMProgramCategory::piano,               21, 108, false },
    GMProgram{  5, "Electric Piano 2",           GMProgramCategory::piano,               21, 108, false },
    GMProgram{  6, "Harpsichord",                GMProgramCategory::piano,               21, 108, false },
    GMProgram{  7, "Clavinet",                   GMProgramCategory::piano,               21, 108, false },
    // 8-15 Chromatic Percussion
    GMProgram{  8, "Celesta",                   GMProgramCategory::chromatic_percussion, 60, 84,  false },
    GMProgram{  9, "Glockenspiel",              GMProgramCategory::chromatic_percussion, 60, 84,  false },
    GMProgram{ 10, "Music Box",                 GMProgramCategory::chromatic_percussion, 60, 84,  false },
    GMProgram{ 11, "Vibraphone",                GMProgramCategory::chromatic_percussion, 53, 89,  false },
    GMProgram{ 12, "Marimba",                   GMProgramCategory::chromatic_percussion, 48, 84,  false },
    GMProgram{ 13, "Xylophone",                 GMProgramCategory::chromatic_percussion, 60, 96,  false },
    GMProgram{ 14, "Tubular Bells",             GMProgramCategory::chromatic_percussion, 48, 84,  false },
    GMProgram{ 15, "Dulcimer",                  GMProgramCategory::chromatic_percussion, 48, 84,  false },
    // 16-23 Organ
    GMProgram{ 16, "Drawbar Organ",             GMProgramCategory::organ,                36, 96,  false },
    GMProgram{ 17, "Percussive Organ",          GMProgramCategory::organ,                36, 96,  false },
    GMProgram{ 18, "Rock Organ",                GMProgramCategory::organ,                36, 96,  false },
    GMProgram{ 19, "Church Organ",              GMProgramCategory::organ,                36, 96,  false },
    GMProgram{ 20, "Reed Organ",                GMProgramCategory::organ,                36, 96,  false },
    GMProgram{ 21, "Accordion",                 GMProgramCategory::organ,                24, 96,  false },
    GMProgram{ 22, "Harmonica",                 GMProgramCategory::organ,                48, 84,  false },
    GMProgram{ 23, "Tango Accordion",           GMProgramCategory::organ,                24, 96,  false },
    // 24-31 Guitar
    GMProgram{ 24, "Nylon String Guitar",       GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 25, "Steel String Guitar",       GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 26, "Electric Jazz Guitar",      GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 27, "Electric Clean Guitar",     GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 28, "Electric Muted Guitar",     GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 29, "Overdriven Guitar",         GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 30, "Distortion Guitar",         GMProgramCategory::guitar,               40, 88,  false },
    GMProgram{ 31, "Guitar Harmonics",          GMProgramCategory::guitar,               40, 88,  false },
    // 32-39 Bass
    GMProgram{ 32, "Acoustic Bass",             GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 33, "Electric Bass (Finger)",    GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 34, "Electric Bass (Pick)",      GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 35, "Fretless Bass",             GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 36, "Slap Bass 1",               GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 37, "Slap Bass 2",               GMProgramCategory::bass,                 28, 60,  false },
    GMProgram{ 38, "Synth Bass 1",              GMProgramCategory::bass,                 28, 72,  false },
    GMProgram{ 39, "Synth Bass 2",              GMProgramCategory::bass,                 28, 72,  false },
    // 40-47 Strings
    GMProgram{ 40, "Violin",                    GMProgramCategory::strings,              48, 100, false },
    GMProgram{ 41, "Viola",                     GMProgramCategory::strings,              36, 88,  false },
    GMProgram{ 42, "Cello",                     GMProgramCategory::strings,              36, 84,  false },
    GMProgram{ 43, "Contrabass",                GMProgramCategory::strings,              28, 60,  false },
    GMProgram{ 44, "Tremolo Strings",           GMProgramCategory::strings,              36, 96,  false },
    GMProgram{ 45, "Pizzicato Strings",         GMProgramCategory::strings,              36, 96,  false },
    GMProgram{ 46, "Orchestral Harp",           GMProgramCategory::strings,              36, 96,  false },
    GMProgram{ 47, "Timpani",                   GMProgramCategory::strings,              36, 60,  false },
    // 48-55 Ensemble
    GMProgram{ 48, "String Ensemble 1",         GMProgramCategory::ensemble,             36, 96,  false },
    GMProgram{ 49, "String Ensemble 2",         GMProgramCategory::ensemble,             36, 96,  false },
    GMProgram{ 50, "Synth Strings 1",           GMProgramCategory::ensemble,             36, 96,  false },
    GMProgram{ 51, "Synth Strings 2",           GMProgramCategory::ensemble,             36, 96,  false },
    GMProgram{ 52, "Choir Aahs",                GMProgramCategory::ensemble,             48, 84,  false },
    GMProgram{ 53, "Voice Oohs",                GMProgramCategory::ensemble,             48, 84,  false },
    GMProgram{ 54, "Synth Voice",               GMProgramCategory::ensemble,             36, 96,  false },
    GMProgram{ 55, "Orchestra Hit",             GMProgramCategory::ensemble,             36, 96,  false },
    // 56-63 Brass
    GMProgram{ 56, "Trumpet",                   GMProgramCategory::brass,                58, 84,  false },
    GMProgram{ 57, "Trombone",                  GMProgramCategory::brass,                40, 72,  false },
    GMProgram{ 58, "Tuba",                      GMProgramCategory::brass,                28, 60,  false },
    GMProgram{ 59, "Muted Trumpet",             GMProgramCategory::brass,                58, 84,  false },
    GMProgram{ 60, "French Horn",               GMProgramCategory::brass,                36, 72,  false },
    GMProgram{ 61, "Brass Section",             GMProgramCategory::brass,                40, 84,  false },
    GMProgram{ 62, "Synth Brass 1",             GMProgramCategory::brass,                36, 84,  false },
    GMProgram{ 63, "Synth Brass 2",             GMProgramCategory::brass,                36, 84,  false },
    // 64-71 Reed
    GMProgram{ 64, "Soprano Sax",               GMProgramCategory::reed,                 54, 84,  false },
    GMProgram{ 65, "Alto Sax",                  GMProgramCategory::reed,                 42, 78,  false },
    GMProgram{ 66, "Tenor Sax",                 GMProgramCategory::reed,                 36, 72,  false },
    GMProgram{ 67, "Baritone Sax",              GMProgramCategory::reed,                 28, 66,  false },
    GMProgram{ 68, "Oboe",                      GMProgramCategory::reed,                 55, 84,  false },
    GMProgram{ 69, "English Horn",              GMProgramCategory::reed,                 54, 78,  false },
    GMProgram{ 70, "Bassoon",                   GMProgramCategory::reed,                 34, 66,  false },
    GMProgram{ 71, "Clarinet",                  GMProgramCategory::reed,                 50, 84,  false },
    // 72-79 Pipe
    GMProgram{ 72, "Piccolo",                   GMProgramCategory::pipe,                 60, 96,  false },
    GMProgram{ 73, "Flute",                     GMProgramCategory::pipe,                 60, 96,  false },
    GMProgram{ 74, "Recorder",                  GMProgramCategory::pipe,                 60, 84,  false },
    GMProgram{ 75, "Pan Flute",                 GMProgramCategory::pipe,                 60, 96,  false },
    GMProgram{ 76, "Blown Bottle",              GMProgramCategory::pipe,                 48, 84,  false },
    GMProgram{ 77, "Shakuhachi",                GMProgramCategory::pipe,                 60, 84,  false },
    GMProgram{ 78, "Whistle",                   GMProgramCategory::pipe,                 72, 96,  false },
    GMProgram{ 79, "Ocarina",                   GMProgramCategory::pipe,                 60, 84,  false },
    // 80-87 Synth Lead
    GMProgram{ 80, "Lead 1 (Square)",           GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 81, "Lead 2 (Sawtooth)",         GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 82, "Lead 3 (Calliope)",         GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 83, "Lead 4 (Chiff)",            GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 84, "Lead 5 (Charang)",          GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 85, "Lead 6 (Voice)",            GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 86, "Lead 7 (Fifths)",           GMProgramCategory::synth_lead,           36, 96,  false },
    GMProgram{ 87, "Lead 8 (Bass + Lead)",      GMProgramCategory::synth_lead,           36, 96,  false },
    // 88-95 Synth Pad
    GMProgram{ 88, "Pad 1 (New Age)",           GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 89, "Pad 2 (Warm)",              GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 90, "Pad 3 (Polysynth)",         GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 91, "Pad 4 (Choir)",             GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 92, "Pad 5 (Bowed)",             GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 93, "Pad 6 (Metallic)",          GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 94, "Pad 7 (Halo)",              GMProgramCategory::synth_pad,            24, 96,  false },
    GMProgram{ 95, "Pad 8 (Sweep)",             GMProgramCategory::synth_pad,            24, 96,  false },
    // 96-103 Synth Effects
    GMProgram{ 96, "FX 1 (Rain)",               GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{ 97, "FX 2 (Soundtrack)",         GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{ 98, "FX 3 (Crystal)",            GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{ 99, "FX 4 (Atmosphere)",         GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{100, "FX 5 (Brightness)",         GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{101, "FX 6 (Goblins)",            GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{102, "FX 7 (Echoes)",             GMProgramCategory::synth_effects,        24, 96,  false },
    GMProgram{103, "FX 8 (Sci-Fi)",             GMProgramCategory::synth_effects,        24, 96,  false },
    // 104-111 Ethnic
    GMProgram{104, "Sitar",                     GMProgramCategory::ethnic,               48, 96,  false },
    GMProgram{105, "Banjo",                     GMProgramCategory::ethnic,               48, 84,  false },
    GMProgram{106, "Shamisen",                  GMProgramCategory::ethnic,               48, 84,  false },
    GMProgram{107, "Koto",                      GMProgramCategory::ethnic,               48, 84,  false },
    GMProgram{108, "Kalimba",                   GMProgramCategory::ethnic,               48, 84,  false },
    GMProgram{109, "Bagpipe",                   GMProgramCategory::ethnic,               48, 84,  false },
    GMProgram{110, "Fiddle",                    GMProgramCategory::ethnic,               36, 96,  false },
    GMProgram{111, "Shanai",                    GMProgramCategory::ethnic,               48, 84,  false },
    // 112-119 Percussion
    GMProgram{112, "Tinkle Bell",               GMProgramCategory::percussion,           60, 96,  false },
    GMProgram{113, "Agogo",                     GMProgramCategory::percussion,           60, 84,  false },
    GMProgram{114, "Steel Drums",              GMProgramCategory::percussion,           48, 84,  false },
    GMProgram{115, "Woodblock",                 GMProgramCategory::percussion,           60, 84,  false },
    GMProgram{116, "Taiko Drum",               GMProgramCategory::percussion,           36, 60,  false },
    GMProgram{117, "Melodic Tom",               GMProgramCategory::percussion,           48, 72,  false },
    GMProgram{118, "Synth Drum",                GMProgramCategory::percussion,           36, 72,  false },
    GMProgram{119, "Reverse Cymbal",            GMProgramCategory::percussion,           36, 72,  false },
    // 120-127 Sound Effects
    GMProgram{120, "Guitar Fret Noise",        GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{121, "Breath Noise",              GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{122, "Seashore",                  GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{123, "Bird Tweet",                GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{124, "Telephone Ring",            GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{125, "Helicopter",                GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{126, "Applause",                  GMProgramCategory::sound_effects,         0,  0,   false },
    GMProgram{127, "Gunshot",                   GMProgramCategory::sound_effects,         0,  0,   false },
};

// GM drum note names (notes 35-81 are standard GM drum sounds).
// Returned as a pair of (note, name_view). Empty string_view for unmapped notes.
struct DrumNote {
    int             note;
    std::string_view name;
};

inline constexpr std::array kDrumNotes = {
    DrumNote{35, "Acoustic Bass Drum"},
    DrumNote{36, "Bass Drum 1"},
    DrumNote{37, "Side Stick"},
    DrumNote{38, "Acoustic Snare"},
    DrumNote{39, "Hand Clap"},
    DrumNote{40, "Electric Snare"},
    DrumNote{41, "Low Floor Tom"},
    DrumNote{42, "Closed Hi-Hat"},
    DrumNote{43, "High Floor Tom"},
    DrumNote{44, "Pedal Hi-Hat"},
    DrumNote{45, "Low Tom"},
    DrumNote{46, "Open Hi-Hat"},
    DrumNote{47, "Low-Mid Tom"},
    DrumNote{48, "Hi-Mid Tom"},
    DrumNote{49, "Crash Cymbal 1"},
    DrumNote{50, "High Tom"},
    DrumNote{51, "Ride Cymbal 1"},
    DrumNote{52, "Chinese Cymbal"},
    DrumNote{53, "Ride Bell"},
    DrumNote{54, "Tambourine"},
    DrumNote{55, "Splash Cymbal"},
    DrumNote{56, "Cowbell"},
    DrumNote{57, "Crash Cymbal 2"},
    DrumNote{58, "Vibraslap"},
    DrumNote{59, "Ride Cymbal 2"},
    DrumNote{60, "Hi Bongo"},
    DrumNote{61, "Low Bongo"},
    DrumNote{62, "Mute Hi Conga"},
    DrumNote{63, "Open Hi Conga"},
    DrumNote{64, "Low Conga"},
    DrumNote{65, "High Timbale"},
    DrumNote{66, "Low Timbale"},
    DrumNote{67, "High Agogo"},
    DrumNote{68, "Low Agogo"},
    DrumNote{69, "Cabasa"},
    DrumNote{70, "Maracas"},
    DrumNote{71, "Short Whistle"},
    DrumNote{72, "Long Whistle"},
    DrumNote{73, "Short Guiro"},
    DrumNote{74, "Long Guiro"},
    DrumNote{75, "Claves"},
    DrumNote{76, "Hi Wood Block"},
    DrumNote{77, "Low Wood Block"},
    DrumNote{78, "Mute Cuica"},
    DrumNote{79, "Open Cuica"},
    DrumNote{80, "Mute Triangle"},
    DrumNote{81, "Open Triangle"},
};

} // namespace detail

// ── Public API ─────────────────────────────────────────────────────

// Get GM program info by program number (0-127). Out-of-range returns program 0.
inline constexpr const GMProgram& get_gm_program(int program) {
    if (program < 0 || program > 127)
        return detail::kGmPrograms[0];
    return detail::kGmPrograms[program];
}

// Get all programs in a category.
// Each category occupies a contiguous block of 8 programs.
inline constexpr std::span<const GMProgram> get_gm_programs_by_category(GMProgramCategory cat) {
    const auto idx = static_cast<int>(cat);
    const auto start = detail::kCategoryStarts[idx];
    const auto count = detail::kProgramsPerCategory;
    return {detail::kGmPrograms.data() + start, static_cast<std::size_t>(count)};
}

// Get the GM drum note name for a MIDI note (35-81).
// Returns empty string_view for unmapped notes.
inline constexpr std::string_view get_gm_drum_name(int note) {
    for (const auto& dn : detail::kDrumNotes) {
        if (dn.note == note)
            return dn.name;
    }
    return {};
}

} // namespace aimidi::theory
