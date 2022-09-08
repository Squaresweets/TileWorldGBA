# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.20

# compile ASM with D:/Program Files/GBU Arm Embedded Toolchain/bin/arm-none-eabi-gcc.exe
# compile C with D:/Program Files/GBU Arm Embedded Toolchain/bin/arm-none-eabi-gcc.exe
# compile CXX with D:/Program Files/GBU Arm Embedded Toolchain/bin/arm-none-eabi-g++.exe
ASM_DEFINES = -DCFG_TUSB_DEBUG=1 -DCFG_TUSB_MCU=OPT_MCU_RP2040 -DCFG_TUSB_OS=OPT_OS_PICO -DCYW43_LWIP=1 -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_CYW43_ARCH=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UNIQUE_ID=1 -DLIB_PICO_UTIL=1 -DLIB_TINYUSB_BOARD=1 -DLIB_TINYUSB_DEVICE=1 -DPICO_BOARD=\"pico_w\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Debug\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_CYW43_ARCH_POLL=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_TARGET_NAME=\"pico\" -DPICO_USE_BLOCKED_RAM=0

ASM_INCLUDES = -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2 -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_stdlib\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_gpio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_base\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\generated\pico_base -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\boards\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_platform\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_regs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_base\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_structs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_claim\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_irq\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_time\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_timer\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_util\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_runtime\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_clocks\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_resets\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pll\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_vreg\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_watchdog\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_xosc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_printf\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_bootrom\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_bit_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_double\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_int64_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_float\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_malloc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\boot_stage2\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_binary_info\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_cyw43_arch\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_unique_id\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_flash\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\pico-sdk\src\rp2_common\cyw43_driver -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\firmware -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_dma\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_exception\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\lwip\src\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_lwip\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src\common -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\hw -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_fix\rp2040_usb_device_enumeration\include

ASM_FLAGS = -mcpu=cortex-m0plus -mthumb -Og -g -ffunction-sections -fdata-sections

C_DEFINES = -DCFG_TUSB_DEBUG=1 -DCFG_TUSB_MCU=OPT_MCU_RP2040 -DCFG_TUSB_OS=OPT_OS_PICO -DCYW43_LWIP=1 -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_CYW43_ARCH=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UNIQUE_ID=1 -DLIB_PICO_UTIL=1 -DLIB_TINYUSB_BOARD=1 -DLIB_TINYUSB_DEVICE=1 -DPICO_BOARD=\"pico_w\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Debug\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_CYW43_ARCH_POLL=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_TARGET_NAME=\"pico\" -DPICO_USE_BLOCKED_RAM=0

C_INCLUDES = -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2 -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_stdlib\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_gpio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_base\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\generated\pico_base -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\boards\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_platform\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_regs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_base\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_structs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_claim\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_irq\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_time\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_timer\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_util\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_runtime\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_clocks\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_resets\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pll\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_vreg\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_watchdog\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_xosc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_printf\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_bootrom\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_bit_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_double\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_int64_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_float\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_malloc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\boot_stage2\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_binary_info\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_cyw43_arch\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_unique_id\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_flash\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\pico-sdk\src\rp2_common\cyw43_driver -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\firmware -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_dma\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_exception\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\lwip\src\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_lwip\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src\common -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\hw -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_fix\rp2040_usb_device_enumeration\include

C_FLAGS = -mcpu=cortex-m0plus -mthumb -Og -g -ffunction-sections -fdata-sections -std=gnu11

CXX_DEFINES = -DCFG_TUSB_DEBUG=1 -DCFG_TUSB_MCU=OPT_MCU_RP2040 -DCFG_TUSB_OS=OPT_OS_PICO -DCYW43_LWIP=1 -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_CYW43_ARCH=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_HARDWARE=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_PICO=1 -DLIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_PICO=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_PICO=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_PICO=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CORE=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_UNIQUE_ID=1 -DLIB_PICO_UTIL=1 -DLIB_TINYUSB_BOARD=1 -DLIB_TINYUSB_DEVICE=1 -DPICO_BOARD=\"pico_w\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Debug\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_CYW43_ARCH_POLL=1 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_TARGET_NAME=\"pico\" -DPICO_USE_BLOCKED_RAM=0

CXX_INCLUDES = -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2 -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_stdlib\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_gpio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_base\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\generated\pico_base -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\boards\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_platform\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_regs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_base\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2040\hardware_structs\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_claim\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_irq\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_sync\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_time\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_timer\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_util\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_runtime\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_clocks\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_resets\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pll\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_vreg\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_watchdog\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_xosc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_printf\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_bootrom\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_bit_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_divider\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_double\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_int64_ops\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_float\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_malloc\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\boot_stage2\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\common\pico_binary_info\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_stdio_uart\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_cyw43_arch\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_unique_id\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_flash\include -IC:\Users\Daniel\Desktop\GBAWork\TileWorldGBA\Pico\configTest2\build\pico-sdk\src\rp2_common\cyw43_driver -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\cyw43-driver\firmware -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_pio\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_dma\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\hardware_exception\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\lwip\src\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_lwip\include -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\src\common -IC:\Users\Daniel\Desktop\pico\pico-sdk\lib\tinyusb\hw -IC:\Users\Daniel\Desktop\pico\pico-sdk\src\rp2_common\pico_fix\rp2040_usb_device_enumeration\include

CXX_FLAGS = -mcpu=cortex-m0plus -mthumb -Og -g -ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables -fno-rtti -fno-use-cxa-atexit -std=gnu++17

