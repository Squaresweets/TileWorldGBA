cmake_minimum_required(VERSION 3.13)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(configTest)
 
pico_sdk_init()

add_executable(configTest)

target_include_directories(configTest PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(configTest PRIVATE
        main.c
        msc_disk.c
        usb_descriptors.c
        )
        
target_link_libraries(configTest PRIVATE pico_stdlib tinyusb_device tinyusb_board)
pico_add_extra_outputs(configTest)

