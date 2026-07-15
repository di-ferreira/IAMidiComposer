#include <aimidi/theory/GMPrograms.hpp>
#include <gtest/gtest.h>

using namespace aimidi::theory;

TEST(GMProgramsTest, AllProgramsAccessible) {
    for (int i = 0; i < 128; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_FALSE(prog.name.empty());
        EXPECT_GE(prog.program, 0);
        EXPECT_LE(prog.program, 127);
    }
}

TEST(GMProgramsTest, PianoCategory) {
    for (int i = 0; i < 8; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::piano);
    }
}

TEST(GMProgramsTest, ChromaticPercussionCategory) {
    for (int i = 8; i < 16; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::chromatic_percussion);
    }
}

TEST(GMProgramsTest, OrganCategory) {
    for (int i = 16; i < 24; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::organ);
    }
}

TEST(GMProgramsTest, GuitarCategory) {
    for (int i = 24; i < 32; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::guitar);
    }
}

TEST(GMProgramsTest, BassCategory) {
    for (int i = 32; i < 40; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::bass);
    }
}

TEST(GMProgramsTest, StringsCategory) {
    for (int i = 40; i < 48; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::strings);
    }
}

TEST(GMProgramsTest, EnsembleCategory) {
    for (int i = 48; i < 56; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::ensemble);
    }
}

TEST(GMProgramsTest, BrassCategory) {
    for (int i = 56; i < 64; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::brass);
    }
}

TEST(GMProgramsTest, ReedCategory) {
    for (int i = 64; i < 72; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::reed);
    }
}

TEST(GMProgramsTest, PipeCategory) {
    for (int i = 72; i < 80; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::pipe);
    }
}

TEST(GMProgramsTest, SynthLeadCategory) {
    for (int i = 80; i < 88; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::synth_lead);
    }
}

TEST(GMProgramsTest, SynthPadCategory) {
    for (int i = 88; i < 96; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::synth_pad);
    }
}

TEST(GMProgramsTest, SynthEffectsCategory) {
    for (int i = 96; i < 104; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::synth_effects);
    }
}

TEST(GMProgramsTest, EthnicCategory) {
    for (int i = 104; i < 112; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::ethnic);
    }
}

TEST(GMProgramsTest, PercussionCategory) {
    for (int i = 112; i < 120; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::percussion);
    }
}

TEST(GMProgramsTest, SoundEffectsCategory) {
    for (int i = 120; i < 128; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_EQ(prog.category, GMProgramCategory::sound_effects);
    }
}

TEST(GMProgramsTest, AcousticGrandPiano) {
    const auto& prog = get_gm_program(0);
    EXPECT_EQ(prog.name, "Acoustic Grand Piano");
    EXPECT_EQ(prog.category, GMProgramCategory::piano);
}

TEST(GMProgramsTest, OverdrivenGuitar) {
    const auto& prog = get_gm_program(29);
    EXPECT_EQ(prog.name, "Overdriven Guitar");
    EXPECT_EQ(prog.category, GMProgramCategory::guitar);
}

TEST(GMProgramsTest, Violin) {
    const auto& prog = get_gm_program(40);
    EXPECT_EQ(prog.name, "Violin");
    EXPECT_EQ(prog.category, GMProgramCategory::strings);
}

TEST(GMProgramsTest, Gunshot) {
    const auto& prog = get_gm_program(127);
    EXPECT_EQ(prog.name, "Gunshot");
    EXPECT_EQ(prog.category, GMProgramCategory::sound_effects);
}

TEST(GMProgramsTest, OutOfRangeReturnsDefault) {
    const auto& prog = get_gm_program(200);
    EXPECT_EQ(prog.name, "Acoustic Grand Piano");
    const auto& prog2 = get_gm_program(-1);
    EXPECT_EQ(prog2.name, "Acoustic Grand Piano");
}

TEST(GMProgramsTest, NoProgramIsDrum) {
    // No GM program 0-127 has is_drum=true (drums are channel-based, not program-based)
    for (int i = 0; i < 128; ++i) {
        const auto& prog = get_gm_program(i);
        EXPECT_FALSE(prog.is_drum) << "Program " << i << " should not be a drum";
    }
}

TEST(GMProgramsTest, GetProgramsByCategoryPiano) {
    auto progs = get_gm_programs_by_category(GMProgramCategory::piano);
    EXPECT_EQ(progs.size(), 8u);
    EXPECT_EQ(progs[0].name, "Acoustic Grand Piano");
    EXPECT_EQ(progs[7].name, "Clavinet");
}

TEST(GMProgramsTest, GetProgramsByCategoryBrass) {
    auto progs = get_gm_programs_by_category(GMProgramCategory::brass);
    EXPECT_EQ(progs.size(), 8u);
    EXPECT_EQ(progs[0].name, "Trumpet");
    EXPECT_EQ(progs[7].name, "Synth Brass 2");
}

TEST(GMProgramsTest, DrumNoteNames) {
    auto name = get_gm_drum_name(36);
    EXPECT_FALSE(name.empty());
}

TEST(GMProgramsTest, DrumNote36) {
    auto name = get_gm_drum_name(36);
    EXPECT_EQ(name, "Bass Drum 1");
}

TEST(GMProgramsTest, DrumNote38) {
    auto name = get_gm_drum_name(38);
    EXPECT_EQ(name, "Acoustic Snare");
}

TEST(GMProgramsTest, DrumNote42) {
    auto name = get_gm_drum_name(42);
    EXPECT_EQ(name, "Closed Hi-Hat");
}

TEST(GMProgramsTest, DrumNote46) {
    auto name = get_gm_drum_name(46);
    EXPECT_EQ(name, "Open Hi-Hat");
}

TEST(GMProgramsTest, DrumNote49) {
    auto name = get_gm_drum_name(49);
    EXPECT_EQ(name, "Crash Cymbal 1");
}

TEST(GMProgramsTest, DrumNote51) {
    auto name = get_gm_drum_name(51);
    EXPECT_EQ(name, "Ride Cymbal 1");
}

TEST(GMProgramsTest, AllDrumNotesHaveNames) {
    for (int note = 35; note <= 81; ++note) {
        auto name = get_gm_drum_name(note);
        EXPECT_FALSE(name.empty()) << "Drum note " << note << " should have a name";
    }
}

TEST(GMProgramsTest, InvalidDrumNote) {
    auto name = get_gm_drum_name(0);
    EXPECT_TRUE(name.empty());
    name = get_gm_drum_name(127);
    EXPECT_TRUE(name.empty());
    name = get_gm_drum_name(34);
    EXPECT_TRUE(name.empty());
    name = get_gm_drum_name(82);
    EXPECT_TRUE(name.empty());
}

TEST(GMProgramsTest, AllProgramNamesUnique) {
    for (int i = 0; i < 128; ++i) {
        for (int j = i + 1; j < 128; ++j) {
            // It's OK for "Gunshot" and some others to be unique just check empty
            EXPECT_NE(get_gm_program(i).name, get_gm_program(j).name)
                << "Program " << i << " and " << j << " share the same name";
        }
    }
}

TEST(GMProgramsTest, ProgramNumbersMatchIndex) {
    for (int i = 0; i < 128; ++i) {
        EXPECT_EQ(get_gm_program(i).program, i);
    }
}
