cmake_minimum_required(VERSION 3.13)

set(NAME pico)
set(PICO_BOARD pico_w)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(${NAME} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
add_executable(pico
        blink.c
        )

# pull in common dependencies
target_link_libraries(pico pico_stdlib hardware_flash)

pico_enable_stdio_usb(pico 1)
pico_enable_stdio_uart(pico 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico)
