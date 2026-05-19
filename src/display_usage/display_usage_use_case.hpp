#pragma once

#include "display_usage/display_out_port.hpp"
#include "display_usage/usage_in_port.hpp"

class DisplayUsageUseCase : public UsageInPort {
  public:
    explicit DisplayUsageUseCase(DisplayOutPort& display);

    void handle_waiting() const;
    void handle_usage_update(const LlmUsage& usage) const override;

  private:
    DisplayOutPort& display_;
};
