cmake_minimum_required(VERSION 3.13)

# 動態取得使用者家目錄（跨機器，不寫死帳號）
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

# Raspberry Pi Pico VS Code 擴充（若有安裝，會自動帶工具鏈/SDK）
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# 開發板：Pico 2 W（RP2350，內建 Wi-Fi）
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pico SDK（需設環境變數 PICO_SDK_PATH，或由 VS Code 擴充提供）
include(pico_sdk_import.cmake)

project(Fire-Camera-System C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# ── FreeRTOS：使用 Raspberry Pi 官方 fork 的 RP2350_ARM_NTZ port ──
# 請設環境變數 FREERTOS_KERNEL_PATH 指向 https://github.com/raspberrypi/FreeRTOS-Kernel
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include")
include("$ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake")

# ── DHT 感測器函式庫（vmilea/pico_dht，PIO 驅動）──
# 請 clone https://github.com/vmilea/pico_dht 到專案根目錄
add_subdirectory(pico_dht/dht)

add_executable(Fire-Camera-System
    src/main.c
    src/network_task.c
    src/task_sensor.c
    src/task_alarm.c
    src/task_logger.c
    src/task_sound.c
    src/web_files.cpp
)

target_include_directories(Fire-Camera-System PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
    "${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include"
    "${PICO_SDK_PATH}/lib/lwip/contrib/ports/freertos/include"
)

target_link_libraries(Fire-Camera-System PRIVATE
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    dht
    hardware_exception
    hardware_gpio
    hardware_adc
)

# stdio 走 USB（printf 透過 USB 序列埠輸出）
pico_enable_stdio_uart(Fire-Camera-System 0)
pico_enable_stdio_usb(Fire-Camera-System 1)
pico_add_extra_outputs(Fire-Camera-System)
