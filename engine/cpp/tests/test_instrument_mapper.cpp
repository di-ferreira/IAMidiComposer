#include <aimidi/theory/IInstrumentMapper.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(InstrumentMapperTest, MapsPiano) {
    auto mapper = make_instrument_mapper();
    ASSERT_TRUE(mapper);
    auto prog = mapper->map_to_gm("piano", "comping");
    EXPECT_EQ(prog.program, 1);
    EXPECT_EQ(prog.family, "Piano");
}

TEST(InstrumentMapperTest, MapsBass) {
    auto mapper = make_instrument_mapper();
    auto prog = mapper->map_to_gm("bass", "bass");
    EXPECT_EQ(prog.program, 34);
}

TEST(InstrumentMapperTest, MapsDrums) {
    auto mapper = make_instrument_mapper();
    auto prog = mapper->map_to_gm("drums", "drum");
    EXPECT_EQ(prog.program, 1);
}

TEST(InstrumentMapperTest, ReplaceInstrument) {
    auto mapper = make_instrument_mapper();
    BlueprintInstrument source{"piano", "comping", 0, 1};
    auto result = mapper->replace_instrument(source, "bass", "bass");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.target.name, "bass");
    EXPECT_EQ(result.target.program, 34);
}

TEST(InstrumentMapperTest, FamiliesNonEmpty) {
    auto mapper = make_instrument_mapper();
    auto families = mapper->families();
    EXPECT_FALSE(families.empty());
}

TEST(InstrumentMapperTest, ProgramsForFamily) {
    auto mapper = make_instrument_mapper();
    auto progs = mapper->programs_for_family("Piano");
    EXPECT_FALSE(progs.empty());
    EXPECT_EQ(progs[0].program, 1);
}
