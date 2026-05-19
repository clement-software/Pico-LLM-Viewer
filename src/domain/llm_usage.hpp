#pragma once

#include <cstdint>
#include <string>

enum class AccessStatus : std::uint8_t { allowed, limited };

struct LlmUsage {
    int session_percentage;
    int session_reset_minutes;
    int weekly_percentage;
    int weekly_reset_minutes;
    std::string status_label;
    AccessStatus access_status;
};
