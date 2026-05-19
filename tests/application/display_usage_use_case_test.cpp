#include <gtest/gtest.h>
#include <optional>
#include "display_usage/display_usage_use_case.hpp"

class FakeDisplay : public DisplayOutPort {
public:
    void show_waiting() override {
        waiting_call_count++;
    }

    void show_usage(const LlmUsage& usage) override {
        last_received_usage = usage;
    }

    int waiting_call_count = 0;
    std::optional<LlmUsage> last_received_usage;
};

TEST(DisplayUsageUseCase, handle_waiting_notifies_display_exactly_once) {
    FakeDisplay display;
    DisplayUsageUseCase use_case(display);

    use_case.handle_waiting();

    EXPECT_EQ(display.waiting_call_count, 1);
}

TEST(DisplayUsageUseCase, handle_waiting_called_multiple_times_notifies_display_each_time) {
    FakeDisplay display;
    DisplayUsageUseCase use_case(display);

    use_case.handle_waiting();
    use_case.handle_waiting();
    use_case.handle_waiting();

    EXPECT_EQ(display.waiting_call_count, 3);
}

TEST(DisplayUsageUseCase, handle_usage_update_forwards_all_usage_fields_to_display) {
    FakeDisplay display;
    DisplayUsageUseCase use_case(display);
    const LlmUsage usage{72, 30, 45, 180, "allowed", AccessStatus::allowed};

    use_case.handle_usage_update(usage);

    ASSERT_TRUE(display.last_received_usage.has_value());
    EXPECT_EQ(display.last_received_usage->session_percentage,    usage.session_percentage);
    EXPECT_EQ(display.last_received_usage->session_reset_minutes, usage.session_reset_minutes);
    EXPECT_EQ(display.last_received_usage->weekly_percentage,     usage.weekly_percentage);
    EXPECT_EQ(display.last_received_usage->weekly_reset_minutes,  usage.weekly_reset_minutes);
    EXPECT_EQ(display.last_received_usage->access_status,         usage.access_status);
}

TEST(DisplayUsageUseCase, handle_usage_update_does_not_trigger_waiting_notification) {
    FakeDisplay display;
    DisplayUsageUseCase use_case(display);
    LlmUsage usage{50, 60, 30, 120, "allowed", AccessStatus::allowed};

    use_case.handle_usage_update(usage);

    EXPECT_EQ(display.waiting_call_count, 0);
}

TEST(DisplayUsageUseCase, successive_usage_updates_display_reflects_the_most_recent_one) {
    FakeDisplay display;
    DisplayUsageUseCase use_case(display);
    const LlmUsage first_usage{10, 60, 5,  120, "allowed",      AccessStatus::allowed};
    const LlmUsage last_usage {90, 5,  85, 10,  "rate_limited", AccessStatus::limited};

    use_case.handle_usage_update(first_usage);
    use_case.handle_usage_update(last_usage);

    ASSERT_TRUE(display.last_received_usage.has_value());
    EXPECT_EQ(display.last_received_usage->session_percentage, last_usage.session_percentage);
    EXPECT_EQ(display.last_received_usage->access_status,      last_usage.access_status);
}
