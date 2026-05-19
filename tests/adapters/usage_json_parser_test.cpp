#include <gtest/gtest.h>
#include "adapters/claude/claude_usage_json_parser.hpp"

TEST(ClaudeUsageJsonParser, returns_no_usage_when_fetch_succeeded_is_false) {
    auto result = ClaudeUsageJsonParser::parse(
        R"({"session_percentage":50,"session_reset_minutes":30,"weekly_percentage":20,"weekly_reset_minutes":60,"status":"allowed","fetch_succeeded":false})"
    );

    EXPECT_FALSE(result.has_value());
}

TEST(ClaudeUsageJsonParser, returns_no_usage_when_json_string_is_empty) {
    EXPECT_FALSE(ClaudeUsageJsonParser::parse("").has_value());
}

TEST(ClaudeUsageJsonParser, parses_all_fields_correctly_when_json_is_valid) {
    constexpr int session_percentage    = 72;
    constexpr int session_reset_minutes = 30;
    constexpr int weekly_percentage     = 45;
    constexpr int weekly_reset_minutes  = 180;

    char json[256];
    snprintf(json, sizeof(json),
        R"({"session_percentage":%d,"session_reset_minutes":%d,"weekly_percentage":%d,"weekly_reset_minutes":%d,"status":"allowed","fetch_succeeded":true})",
        session_percentage, session_reset_minutes, weekly_percentage, weekly_reset_minutes);

    auto result = ClaudeUsageJsonParser::parse(json);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->session_percentage,    session_percentage);
    EXPECT_EQ(result->session_reset_minutes, session_reset_minutes);
    EXPECT_EQ(result->weekly_percentage,     weekly_percentage);
    EXPECT_EQ(result->weekly_reset_minutes,  weekly_reset_minutes);
    EXPECT_EQ(result->status_label,          "allowed");
}

TEST(ClaudeUsageJsonParser, maps_allowed_status_label_to_access_status_allowed) {
    auto result = ClaudeUsageJsonParser::parse(
        R"({"session_percentage":0,"session_reset_minutes":0,"weekly_percentage":0,"weekly_reset_minutes":0,"status":"allowed","fetch_succeeded":true})"
    );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->access_status, AccessStatus::allowed);
}

TEST(ClaudeUsageJsonParser, maps_any_status_label_other_than_allowed_to_access_status_limited) {
    auto result = ClaudeUsageJsonParser::parse(
        R"({"session_percentage":0,"session_reset_minutes":0,"weekly_percentage":0,"weekly_reset_minutes":0,"status":"rate_limited","fetch_succeeded":true})"
    );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->access_status, AccessStatus::limited);
}

TEST(ClaudeUsageJsonParser, defaults_session_reset_minutes_to_minus_one_when_session_reset_minutes_field_is_absent) {
    auto result = ClaudeUsageJsonParser::parse(
        R"({"session_percentage":50,"weekly_percentage":20,"weekly_reset_minutes":60,"status":"allowed","fetch_succeeded":true})"
    );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->session_reset_minutes, -1);
}

TEST(ClaudeUsageJsonParser, defaults_weekly_reset_minutes_to_minus_one_when_weekly_reset_minutes_field_is_absent) {
    auto result = ClaudeUsageJsonParser::parse(
        R"({"session_percentage":50,"session_reset_minutes":10,"weekly_percentage":20,"status":"allowed","fetch_succeeded":true})"
    );

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->weekly_reset_minutes, -1);
}

TEST(ClaudeUsageJsonParser, parses_session_and_weekly_percentage_correctly_at_one_hundred_percent) {
    constexpr int percentage = 100;

    char json[256];
    snprintf(json, sizeof(json),
        R"({"session_percentage":%d,"session_reset_minutes":0,"weekly_percentage":%d,"weekly_reset_minutes":0,"status":"allowed","fetch_succeeded":true})",
        percentage, percentage);

    auto result = ClaudeUsageJsonParser::parse(json);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->session_percentage, percentage);
    EXPECT_EQ(result->weekly_percentage,  percentage);
}

TEST(ClaudeUsageJsonParser, parses_session_and_weekly_percentage_correctly_at_zero_percent) {
    constexpr int percentage = 0;

    char json[256];
    snprintf(json, sizeof(json),
        R"({"session_percentage":%d,"session_reset_minutes":120,"weekly_percentage":%d,"weekly_reset_minutes":360,"status":"allowed","fetch_succeeded":true})",
        percentage, percentage);

    auto result = ClaudeUsageJsonParser::parse(json);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->session_percentage, percentage);
    EXPECT_EQ(result->weekly_percentage,  percentage);
}
