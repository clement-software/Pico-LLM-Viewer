#include "adapters/pico_display/pico_graphics_display.hpp"
#include "adapters/serial/serial_line_reader.hpp"
#include "display_usage/display_usage_use_case.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "pico/stdlib.h"
#include "pico_display.hpp"

using namespace pimoroni;

static uint16_t frame_buffer[PicoDisplay::WIDTH * PicoDisplay::HEIGHT];

int main() {
    stdio_init_all();

    ST7789 st7789_driver(
        PicoDisplay::WIDTH, PicoDisplay::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
    st7789_driver.set_backlight(255);

    PicoGraphics_PenRGB565 graphics(st7789_driver.width, st7789_driver.height, frame_buffer);

    PicoGraphicsDisplay display(st7789_driver, graphics);
    DisplayUsageUseCase use_case(display);
    SerialLineReader serial_reader(use_case);

    use_case.handle_waiting();

    while (true) {
        serial_reader.poll();
    }
}
