#include <aimidi/theory/InputValidation.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace aimidi::theory;

TEST(InputValidationTest, AcceptsValidPrompt) {
    auto result = InputValidation::validate_prompt("upbeat pop song in C major");
    EXPECT_TRUE(result.valid);
    EXPECT_TRUE(result.error.empty());
}

TEST(InputValidationTest, RejectsEmptyPrompt) {
    auto result = InputValidation::validate_prompt("");
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.error.empty());
}

TEST(InputValidationTest, RejectsLiteralNotes) {
    auto result = InputValidation::validate_prompt("play notes C E G");
    EXPECT_FALSE(result.valid);
}

TEST(InputValidationTest, RejectsMidiNoteRequest) {
    auto result = InputValidation::validate_prompt("generate midi notes C4 D4 E4");
    EXPECT_FALSE(result.valid);
}

TEST(InputValidationTest, RejectsWriteNotes) {
    auto result = InputValidation::validate_prompt("write notes for piano");
    EXPECT_FALSE(result.valid);
}

TEST(InputValidationTest, RejectsExplicitChordSpelling) {
    auto result = InputValidation::validate_prompt("c e g chord");
    EXPECT_FALSE(result.valid);
}

TEST(InputValidationTest, AcceptsStyleDescription) {
    auto result = InputValidation::validate_prompt("create a jazz piano trio with walking bass");
    EXPECT_TRUE(result.valid);
}

TEST(InputValidationTest, SanitizeTruncatesLongPrompt) {
    std::string long_prompt(5000, 'a');
    auto result = InputValidation::validate_prompt(long_prompt);
    EXPECT_FALSE(result.valid);
    EXPECT_TRUE(result.error.find("maximum length") != std::string::npos);
}

TEST(InputValidationTest, ForbiddenPatternDetected) {
    EXPECT_TRUE(InputValidation::contains_forbidden_patterns("play note C4"));
    EXPECT_FALSE(InputValidation::contains_forbidden_patterns("play a happy melody"));
}
