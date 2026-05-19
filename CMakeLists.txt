cmake_minimum_required(VERSION 3.12)

set(NAME pico-llm-viewer)

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${NAME}
    main.cpp
    src/display_usage/display_usage_use_case.cpp
    src/adapters/claude/claude_usage_json_parser.cpp
    src/adapters/pico_display/pico_graphics_display.cpp
    src/adapters/serial/serial_line_reader.cpp
)

target_include_directories(${NAME} PRIVATE src)

include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(drivers/st7789/st7789)
include(libraries/pico_graphics/pico_graphics)
include(libraries/pico_display/pico_display)

target_link_libraries(${NAME}
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_gpio
        hardware_pio
        hardware_pwm
        hardware_clocks

        pimoroni_bus
        st7789
        pico_graphics
        pico_display
        bitmap_fonts
)

# USB serial for receiving JSON from the host daemon; UART disabled
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

pico_add_extra_outputs(${NAME})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
