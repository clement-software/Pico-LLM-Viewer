#pragma once

#include "display_usage/usage_in_port.hpp"

#include <array>

class SerialLineReader {
  public:
    explicit SerialLineReader(UsageInPort& handler);

    void poll();

  private:
    static constexpr int line_buffer_capacity = 256;

    UsageInPort& handler_;
    std::array<char, line_buffer_capacity> line_buffer_{};
    int buffer_position_{0};
};
