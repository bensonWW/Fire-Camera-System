cmake_minimum_required(VERSION 3.13)

# 🎯【硬體工具鏈死鎖修正】
set(CMAKE_C_COMPILER "C:/Users/Benson/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-gcc.exe" CACHE FILEPATH "C Compiler" FORCE)
set(CMAKE_CXX_COMPILER "C:/Users/Benson/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-g++.exe" CACHE FILEPATH "CXX Compiler" FORCE)
set(CMAKE_ASM_COMPILER "C:/Users/Benson/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-gcc.exe" CACHE FILEPATH "ASM Compiler" FORCE)

# ====================================================================================
# == 1. Pico 官方 VS Code 擴充功能專用區塊 ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# == 2. 設定開發板與引入 Pico SDK 導入檔 ==
set(PICO_BOARD pico2_w CACHE STRING "Board type" FORCE)
set(PICO_SDK_PATH "C:/Users/Benson/.pico-sdk/sdk/2.2.0" CACHE PATH "" FORCE)

# 🎯 建立 FreeRTOS 官方說明的 Mandatory 配置介面庫
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

# 🎯 設定 FreeRTOS 的硬體架構接口與 Heap 記憶體模式
set(FREERTOS_PORT "GCC_ARM_CM33_NTZ_NONSECURE" CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# 引入 Pico SDK 核心導入檔
include(pico_sdk_import.cmake)

# 執行專案宣告與 Pico SDK 初始化
project(Fire-Camera-System C CXX ASM)
pico_sdk_init()

# 掛載 FreeRTOS 子目錄
add_subdirectory(FreeRTOS-Kernel)

# --- 3. 應用程式原始碼執行檔 ---
add_executable(Fire-Camera-System
    src/main.c
    src/fire_monitor.c
    src/network_task.c
    src/sound_listen.c
)

# 標頭檔搜尋路徑
target_include_directories(Fire-Camera-System PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# --- 4. 連結外部函式庫 ---
target_link_libraries(Fire-Camera-System PRIVATE
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_http                 # 🎯【核心修正】：加入這行以支援 httpd_init 網頁伺服器
    freertos_kernel                  
    hardware_gpio
    hardware_adc
    hardware_spi
    hardware_i2c
)

# 標準設定：開 USB 輸出、關 UART、產生 .uf2 檔案
pico_enable_stdio_uart(Fire-Camera-System 0)
pico_enable_stdio_usb(Fire-Camera-System 1)
pico_add_extra_outputs(Fire-Camera-System)