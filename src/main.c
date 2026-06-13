/* ============================================================
 * Edge-FireCam / 智慧溫度警報系統 — main.c
 * 平台：Raspberry Pi Pico 2 W (RP2350) + FreeRTOS (SMP)
 * ============================================================ */
#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "network_task.h"   /* WiFi + httpd + LED 狀態燈 */
#include "shared.h"         /* 溫度警報子系統共享資料 + 任務 */

/* ── 全域 Kernel 物件（shared.h extern） ── */
QueueHandle_t      xSensorQueue;     /* Sensor → Alarm */
SemaphoreHandle_t  xAlarmSemaphore;  /* Alarm → Logger */
SemaphoreHandle_t  xDataMutex;       /* 保護 gSharedData */
TimerHandle_t      xLogTimer;        /* 每 60 秒清理 log */
SensorData_t       gSharedData = {0};

/* ── Software Timer callback：觸發 Logger 清理 ── */
static void vLogTimerCallback(TimerHandle_t xTimer) {
    (void)xTimer;
    task_logger_trigger_cleanup();
}

/* ── FreeRTOS Hooks ── */
void vApplicationMallocFailedHook(void) {
    printf("\n🔥 [FATAL] FreeRTOS 記憶體分配失敗 (Malloc Failed)！\n");
    for (;;) {}
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    printf("\n🔥 [FATAL] Task [%s] 堆疊溢位 (Stack Overflow)！\n", pcTaskName);
    for (;;) {}
}

int main() {
    /* ⚠️ 排程器啟動前只做 stdio_init_all + 建立物件 + 建立任務 + 啟動排程器。
     *    絕不在此呼叫 sleep_ms / printf / cyw43_arch_init（會 hard fault）。 */
    stdio_init_all();

    /* ── 建立 Kernel 物件 ── */
    xSensorQueue    = xQueueCreate(10, sizeof(SensorData_t));
    xAlarmSemaphore = xSemaphoreCreateBinary();
    xDataMutex      = xSemaphoreCreateMutex();

    xLogTimer = xTimerCreate("LogTimer", pdMS_TO_TICKS(60000),
                             pdTRUE, NULL, vLogTimerCallback);
    xTimerStart(xLogTimer, 0);

    /* ── 建立 Task ──
     *  Alarm(4) > Sensor(3) > Network/WebServer(2) > Led/Logger(1) */
    xTaskCreate(network_task,  "NetworkTask", 2048, NULL, 2, NULL);
    xTaskCreate(led_task,      "LedTask",      512, NULL, 1, NULL);
    xTaskCreate(vTask_Sensor,  "Sensor",       512, NULL, 3, NULL);
    xTaskCreate(vTask_Alarm,   "Alarm",        512, NULL, 4, NULL);
    xTaskCreate(vTask_Sound,   "Sound",        512, NULL, 3, NULL);
    xTaskCreate(vTask_Logger,  "Logger",      1024, NULL, 1, NULL);

    /* ── 啟動排程器（不返回） ── */
    vTaskStartScheduler();

    while (true) {}
    return 0;
}
