cmake_minimum_required(VERSION 3.13)

set(FAMILY rp2040)
set(PICO_BOARD pico_w)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED ENV{WIFI_SSID})
    message(FATAL_ERROR "Environment variable WIFI_SSID not defiened")
endif()

if (NOT DEFINED ENV{WIFI_PASSWORD})
    message(FATAL_ERROR "Environment variable WIFI_PASSWORD not defiened")
endif()

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()


     

#family_initialize_project(test ${CMAKE_CURRENT_LIST_DIR})


add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

set(KISSFFT_DATATYPE float)
set(KISSFFT_TOOLS 0)
set(KISSFFT_TEST OFF)

add_subdirectory(lib/kissfft)

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)


#pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/src/lcd/lcd.pio)
#pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/src/ws2812/ws2812.pio)
pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/src/ws2812/ws2812.pio)


target_sources(test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/out.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usb/usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usb/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cmp/cmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812/ws2812.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812/ws2812_music.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812/ws2812_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net/net.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/packet_mgr/packet_mgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/control/control.c
)

# Example include
target_include_directories(test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usb
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cmp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/packet_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/src/control
)


# create map/bin/hex file etc.
pico_add_extra_outputs(test)

#target_sources(test PRIVATE blink.c)


# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(test PUBLIC pico_stdlib pico_cyw43_arch_lwip_poll pico_multicore pico_sync 
    pico_mem_ops hardware_pio hardware_clocks hardware_interp 
    hardware_dma hardware_pwm hardware_adc hardware_irq hardware_uart
    kissfft tinyusb_device tinyusb_board) # tinyusb_common_base tinyusb_device_base tinyusb_bsp 

# pico_cyw43_arch_lwip_threadsafe_background
target_compile_definitions(test PUBLIC
    WIFI_SSID=\"$ENV{WIFI_SSID}\"
    WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
)

#target_compile_definitions(test PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)
#pico_enable_stdio_usb(${PROJECT_NAME} 0)
#pico_enable_stdio_uart(${PROJECT_NAME} 1)
