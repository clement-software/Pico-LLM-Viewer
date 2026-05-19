#include "adapters/pico_display/pico_graphics_display.hpp"

#include <array>
#include <cstdio>

using namespace pimoroni;

PicoGraphicsDisplay::PicoGraphicsDisplay(ST7789& driver, PicoGraphics_PenRGB565& graphics)
    : driver_{driver}
    , graphics_{graphics} {}

void PicoGraphicsDisplay::set_pen(uint8_t red, uint8_t green, uint8_t blue) {
    graphics_.set_pen(graphics_.create_pen(red, green, blue));
}

void PicoGraphicsDisplay::set_pen_for_percentage(int percentage) {
    if (percentage < 60) {
        set_pen(30, 200, 60);
    } else if (percentage < 80) {
        set_pen(230, 200, 0);
    } else {
        set_pen(220, 40, 40);
    }
}

void PicoGraphicsDisplay::draw_progress_bar(
    int origin_x, int origin_y, int bar_width, int bar_height, int percentage) {
    set_pen(40, 40, 40);
    graphics_.rectangle(Rect(origin_x, origin_y, bar_width, bar_height));

    int filled_width = bar_width * percentage / 100;
    if (filled_width > 0) {
        set_pen_for_percentage(percentage);
        graphics_.rectangle(Rect(origin_x, origin_y, filled_width, bar_height));
    }

    set_pen(100, 100, 100);
    graphics_.rectangle(Rect(origin_x, origin_y, bar_width, 1));
    graphics_.rectangle(Rect(origin_x, origin_y + bar_height - 1, bar_width, 1));
    graphics_.rectangle(Rect(origin_x, origin_y, 1, bar_height));
    graphics_.rectangle(Rect(origin_x + bar_width - 1, origin_y, 1, bar_height));
}

void PicoGraphicsDisplay::format_duration_minutes(char* output_buffer, int minutes) {
    if (minutes < 0) {
        snprintf(output_buffer, 24, "--");
    } else if (minutes < 60) {
        snprintf(output_buffer, 24, "%dmin", minutes);
    } else if (minutes < 1440) {
        snprintf(output_buffer, 24, "%dh%02d", minutes / 60, minutes % 60);
    } else {
        snprintf(output_buffer, 24, "%dd%02dh", minutes / 1440, (minutes % 1440) / 60);
    }
}

void PicoGraphicsDisplay::show_waiting() {
    set_pen(0, 0, 0);
    graphics_.clear();

    set_pen(255, 200, 0);
    graphics_.text("Claude Usage", Point(44, 46), 240, 2.0F);

    set_pen(110, 110, 110);
    graphics_.text("waiting for data...", Point(28, 90), 240, 1.0F);

    driver_.update(&graphics_);
}

void PicoGraphicsDisplay::show_usage(const LlmUsage& usage) {
    set_pen(0, 0, 0);
    graphics_.clear();

    set_pen(255, 255, 255);
    graphics_.text("Claude Usage", Point(44, 4), 240, 2.0F);

    constexpr int bar_x     = 36;
    constexpr int bar_width = 155;

    set_pen_for_percentage(usage.session_percentage);
    graphics_.text("5h", Point(4, 27), 32, 2.0F);

    draw_progress_bar(bar_x, 28, bar_width, 14, usage.session_percentage);

    std::array<char, 32> text_buffer{};
    snprintf(text_buffer.data(), text_buffer.size(), "%d%%", usage.session_percentage);
    set_pen(220, 220, 220);
    graphics_.text(text_buffer.data(), Point(196, 27), 44, 1.0F);

    std::array<char, 24> duration_buffer{};
    format_duration_minutes(duration_buffer.data(), usage.session_reset_minutes);
    snprintf(text_buffer.data(), text_buffer.size(), "reset: %s", duration_buffer.data());
    set_pen(90, 90, 90);
    graphics_.text(text_buffer.data(), Point(bar_x, 46), bar_width, 1.0F);

    set_pen_for_percentage(usage.weekly_percentage);
    graphics_.text("7d", Point(4, 64), 32, 2.0F);

    draw_progress_bar(bar_x, 65, bar_width, 14, usage.weekly_percentage);

    snprintf(text_buffer.data(), text_buffer.size(), "%d%%", usage.weekly_percentage);
    set_pen(220, 220, 220);
    graphics_.text(text_buffer.data(), Point(196, 64), 44, 1.0F);

    format_duration_minutes(duration_buffer.data(), usage.weekly_reset_minutes);
    snprintf(text_buffer.data(), text_buffer.size(), "reset: %s", duration_buffer.data());
    set_pen(90, 90, 90);
    graphics_.text(text_buffer.data(), Point(bar_x, 83), bar_width, 1.0F);

    set_pen(60, 60, 60);
    graphics_.rectangle(Rect(0, 102, 240, 1));

    bool is_limited = (usage.access_status == AccessStatus::limited);
    if (is_limited) {
        set_pen(220, 40, 40);
    } else {
        set_pen(30, 200, 60);
    }
    snprintf(text_buffer.data(), text_buffer.size(), "Status: %s", usage.status_label.c_str());
    graphics_.text(text_buffer.data(), Point(8, 112), 240, 2.0F);

    driver_.update(&graphics_);
}
