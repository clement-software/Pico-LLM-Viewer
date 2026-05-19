#pragma once

#include "domain/llm_usage.hpp"

class DisplayOutPort {
  public:
    DisplayOutPort()                                 = default;
    virtual ~DisplayOutPort()                        = default;
    DisplayOutPort(const DisplayOutPort&)            = delete;
    DisplayOutPort& operator=(const DisplayOutPort&) = delete;
    DisplayOutPort(DisplayOutPort&&)                 = delete;
    DisplayOutPort& operator=(DisplayOutPort&&)      = delete;

    virtual void show_waiting()                    = 0;
    virtual void show_usage(const LlmUsage& usage) = 0;
};
