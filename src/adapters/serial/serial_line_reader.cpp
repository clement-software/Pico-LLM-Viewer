#include "adapters/serial/serial_line_reader.hpp"

#include "adapters/claude/claude_usage_json_parser.hpp"
#include "pico/stdlib.h"

SerialLineReader::SerialLineReader(UsageInPort& handler)
    : handler_{handler} {}

void SerialLineReader::poll() {
    int received_char = getchar_timeout_us(100'000);
    if (received_char == PICO_ERROR_TIMEOUT) {
        return;
    }

    if (received_char == '\n' || received_char == '\r') {
        if (buffer_position_ > 0) {
            line_buffer_.at(buffer_position_) = '\0';
            buffer_position_                  = 0;

            auto parsed_usage = ClaudeUsageJsonParser::parse(line_buffer_.data());
            if (parsed_usage.has_value()) {
                handler_.handle_usage_update(*parsed_usage);
            }
        }
        return;
    }

    if (buffer_position_ < line_buffer_capacity - 1) {
        line_buffer_.at(buffer_position_++) = static_cast<char>(received_char);
    }
}
