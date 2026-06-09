# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion RISCV_RPI_2_0_0_5)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Fire-Camera-System C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# --- FreeRTOS 核心檔案配置 ---
# 定義 FreeRTOS 原始碼路徑
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel")

# 引入你的 FreeRTOS 原始碼檔案
file(GLOB FREERTOS_SOURCES
    "${FREERTOS_KERNEL_PATH}/*.c"
    "${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/*.c" # 適用於 Pico 的 Cortex-M0+ 移植層
    "${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c" # 記憶體管理模組
)

# --- 建立執行檔與編譯路徑 ---
# 將你搬過去的所有 .c 功能檔案全部加進編譯清單
execute_process(COMMAND python ${CMAKE_CURRENT_LIST_DIR}/script/html_to_c.py)
add_executable(Fire-Camera-System 
    src/main.c 
    src/fire_monitor.c 
    src/network_task.c 
    src/sound_listen.c
    ${FREERTOS_SOURCES}
)

pico_set_program_name(Fire-Camera-System "Fire-Camera-System")
pico_set_program_version(Fire-Camera-System "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Fire-Camera-System 0)
pico_enable_stdio_usb(Fire-Camera-System 1)

# --- 連結驅動函式庫 ---
# 包含硬體所需的 GPIO 和 ADC 控制庫
target_link_libraries(Fire-Camera-System
    pico_stdlib
    hardware_gpio
    hardware_adc
)

# --- 標頭檔搜尋路徑 (Include Paths) ---
# 告訴編譯器你的 include 資料夾以及 FreeRTOS 的標頭檔在哪裡
target_include_directories(Fire-Camera-System PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0
)

pico_add_extra_outputs(Fire-Camera-System)


