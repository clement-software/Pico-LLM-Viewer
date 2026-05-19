#pragma once

#include "domain/llm_usage.hpp"

#include <optional>
#include <string_view>

class ClaudeUsageJsonParser {
  public:
    static std::optional<LlmUsage> parse(std::string_view json);
};
