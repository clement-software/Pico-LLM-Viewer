#include "display_usage/display_usage_use_case.hpp"

DisplayUsageUseCase::DisplayUsageUseCase(DisplayOutPort& display)
    : display_{display} {}

void DisplayUsageUseCase::handle_waiting() const {
    display_.show_waiting();
}

void DisplayUsageUseCase::handle_usage_update(const LlmUsage& usage) const {
    display_.show_usage(usage);
}
