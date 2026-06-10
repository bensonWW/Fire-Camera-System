#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/cyw43_arch.h"

// 引入各個任務的標頭檔
#include "network_task.h"
// #include "fire_monitor.h"  // 等你寫好這兩個檔案的宣告後可以解開註解
// #include "sound_listen.h"
void vApplicationMallocFailedHook( void ) {
    printf("\n🔥 [FATAL] FreeRTOS 記憶體分配失敗 (Malloc Failed)！\n");
    printf("🔥 剩餘的 Heap 空間不足以支援目前的 Task 堆疊需求！\n");
    portDISABLE_INTERRUPTS();
    for( ;; );
}

// 🎯【堆疊溢位黑盒子】
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
    printf("\n🔥 [FATAL] Task [%s] 發生堆疊溢位 (Stack Overflow)！\n", pcTaskName);
    printf("🔥 該任務的分配空間爆了，正在污染其他記憶體區域！\n");
    portDISABLE_INTERRUPTS();
    for( ;; );
}
// 預留：火災監控任務體（暫時空著，讓系統能編譯）
void fire_monitor_task(void *pvParameters) {
    printf("[FireMonitor] 火災監控任務已啟動...\n");
    fflush(stdout);
    while (true) {
        // 這裡未來會放讀取 ADC 溫度的 code
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒檢查一次
    }
}

// 預留：聲音聆聽任務體（暫時空著，讓系統能編譯）
void sound_listen_task(void *pvParameters) {
    printf("[SoundListen] 聲音聆聽任務已啟動...\n");
    fflush(stdout);
    while (true) {
        // 這裡未來會放檢查麥克風 GPIO 的 code
        vTaskDelay(pdMS_TO_TICKS(500));  // 每 0.5 秒檢查一次
    }
}

int main() {
    // 1. 初始化 Pico 的標準輸入輸出
    stdio_init_all();
    
    // 等待 Serial Monitor 連線
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("\n============================================\n");
    printf("   🚨 Edge-FireCam 消防相機系統 啟動中... 🚨   \n");
    printf("   執行平台: Raspberry Pi Pico 2 W (FreeRTOS)  \n");
    printf("============================================\n");

    // 🎯【硬體全打通】：在法官（排程器）上場前，先在裸機狀態下把 Wi-Fi 晶片硬體完全準備好！
    printf("[System] 正在初始化 cyw43 無線晶片硬體...\n");
    if (cyw43_arch_init()) {
        printf("[System] ❌ 嚴重錯誤: cyw43 晶片硬體初始化失敗！\n");
        while(1);
    }
    // 開啟 Wi-Fi 站點模式
    cyw43_arch_enable_sta_mode();
    printf("[System] 🎉 cyw43 晶片硬體初始化成功！\n");

    // 2. 建立 FreeRTOS 任務 (Tasks)
    // 🎯 這裡把網路優先度暫時降到 1，讓它跟火災、聲音任務平起平坐，防止飢餓卡死
    // xTaskCreate(network_task, "NetworkTask", 2048, NULL, 1, NULL);
    xTaskCreate(fire_monitor_task, "FireMonitorTask", 1024, NULL, 1, NULL);
    xTaskCreate(sound_listen_task, "SoundListenTask", 1024, NULL, 1, NULL);

    printf("[System] 所有 FreeRTOS 任務建立完畢，啟動排程器...\n");

    // 3. 啟動排程器
    vTaskStartScheduler();

    // 最後防線
    while (true) {}
    return 0;
}