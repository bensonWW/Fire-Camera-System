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
    while (true) {
        // 這裡未來會放讀取 ADC 溫度的 code
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒檢查一次
    }
}

// 預留：聲音聆聽任務體（暫時空著，讓系統能編譯）
void sound_listen_task(void *pvParameters) {
    printf("[SoundListen] 聲音聆聽任務已啟動...\n");
    while (true) {
        // 這裡未來會放檢查麥克風 GPIO 的 code
        vTaskDelay(pdMS_TO_TICKS(500));  // 每 0.5 秒檢查一次
    }
}

int main() {
    stdio_init_all();
    
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("\n============================================\n");
    printf("   🚨 Edge-FireCam 消防相機系統 啟動中... 🚨   \n");
    printf("   執行平台: Raspberry Pi Pico 2 W (FreeRTOS)  \n");
    printf("============================================\n");

    // 🎯 任務優先度一律設為 1，平起平坐
    BaseType_t xTaskStatus;

    xTaskStatus = xTaskCreate(network_task, "NetworkTask", 2048, NULL, 1, NULL);
    if (xTaskStatus != pdPASS) {
        printf("[System] ❌ NetworkTask 建立失敗\n");
        for(;;);
    }

    xTaskStatus = xTaskCreate(fire_monitor_task, "FireMonitorTask", 1024, NULL, 1, NULL);
    if (xTaskStatus != pdPASS) {
        printf("[System] ❌ FireMonitorTask 建立失敗\n");
        for(;;);
    }

    xTaskStatus = xTaskCreate(sound_listen_task, "SoundListenTask", 1024, NULL, 1, NULL);
    if (xTaskStatus != pdPASS) {
        printf("[System] ❌ SoundListenTask 建立失敗\n");
        for(;;);
    }

    printf("[System] 所有 FreeRTOS 任務建立完畢，啟動排程器...\n");
    vTaskStartScheduler();

    printf("[System] ❌ vTaskStartScheduler() 已回傳，排程器啟動失敗！\n");
    for( ;; );
    return 0;
}