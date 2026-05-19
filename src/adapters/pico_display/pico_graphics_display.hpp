#pragma once

#include "display_usage/display_out_port.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"

class PicoGraphicsDisplay final : public DisplayOutPort {
  public:
    PicoGraphicsDisplay(pimoroni::ST7789& driver, pimoroni::PicoGraphics_PenRGB565& graphics);

    void show_waiting() override;
    void show_usage(const LlmUsage& usage) override;

  private:
    void set_pen(uint8_t red, uint8_t green, uint8_t blue);
    void set_pen_for_percentage(int percentage);
    void
    draw_progress_bar(int origin_x, int origin_y, int bar_width, int bar_height, int percentage);
    static void format_duration_minutes(char* output_buffer, int minutes);

    pimoroni::ST7789& driver_;
    pimoroni::PicoGraphics_PenRGB565& graphics_;
};
