#include "adapters/claude/claude_usage_json_parser.hpp"

#include <cstdlib>
#include <cstring>

std::optional<LlmUsage> ClaudeUsageJsonParser::parse(std::string_view json_view) {
    const char* json = json_view.data();

    bool is_ok = strstr(json, "\"fetch_succeeded\":true") != nullptr;
    if (!is_ok) {
        return std::nullopt;
    }

    auto find_int_with_default =
        [](const char* haystack, const char* key, int default_value) -> int {
        const char* found = strstr(haystack, key);
        if (found == nullptr) {
            return default_value;
        }
        const char* start = found + strlen(key);
        char* endptr      = nullptr;
        const long value  = strtol(start, &endptr, 10);
        return (endptr != start) ? static_cast<int>(value) : default_value;
    };

    int session_percentage    = find_int_with_default(json, "\"session_percentage\":", 0);
    int session_reset_minutes = find_int_with_default(json, "\"session_reset_minutes\":", -1);
    int weekly_percentage     = find_int_with_default(json, "\"weekly_percentage\":", 0);
    int weekly_reset_minutes  = find_int_with_default(json, "\"weekly_reset_minutes\":", -1);

    std::string status_label       = "unknown";
    const char* status_value_start = strstr(json, R"("status":")");
    if (status_value_start != nullptr) {
        status_value_start += 10;
        const char* status_value_end = strchr(status_value_start, '"');
        if (status_value_end != nullptr) {
            status_label = std::string(status_value_start, status_value_end);
        }
    }

    AccessStatus access_status =
        (status_label == "allowed") ? AccessStatus::allowed : AccessStatus::limited;

    return LlmUsage{session_percentage,
                    session_reset_minutes,
                    weekly_percentage,
                    weekly_reset_minutes,
                    std::move(status_label),
                    access_status};
}
