cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

set(KISSFFT_DATATYPE int16_t)
set(KISSFFT_TOOLS 0)
set(KISSFFT_TEST OFF)
add_subdirectory(kissfft)

add_executable(test
    slp.c lcd.h lcd.c lcd_graph.h lcd_graph.c lcd_primitives.h lcd_primitives.c lcd_indicator.c lcd_indicator.h fft.h fft.c adc.h adc.c chan.c chan.h
)

#pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/lcd.pio)
# create map/bin/hex file etc.
pico_add_extra_outputs(test)

#target_sources(test PRIVATE blink.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(test PRIVATE pico_stdlib pico_multicore pico_sync pico_mem_ops hardware_pio hardware_interp hardware_dma hardware_adc  hardware_irq kissfft)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
