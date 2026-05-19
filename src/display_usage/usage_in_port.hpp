#pragma once

#include "domain/llm_usage.hpp"

class UsageInPort {
  public:
    UsageInPort()                              = default;
    virtual ~UsageInPort()                     = default;
    UsageInPort(const UsageInPort&)            = delete;
    UsageInPort& operator=(const UsageInPort&) = delete;
    UsageInPort(UsageInPort&&)                 = delete;
    UsageInPort& operator=(UsageInPort&&)      = delete;

    virtual void handle_usage_update(const LlmUsage& usage) const = 0;
};
