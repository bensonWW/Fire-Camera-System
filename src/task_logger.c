/* ============================================================
 * task_logger.c — 環形緩衝記錄歷史/警報事件，Software Timer 定時清理
 * Kernel API：xSemaphoreTake (Binary Sem + Mutex), Software Timer
 * ============================================================ */

#include "shared.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>

#define LOG_MAX_ENTRIES  50   /* 環形緩衝最多 50 筆 */

typedef struct {
    float    temperature;
    float    humidity;
    uint32_t timestamp_ms;
    bool     is_alarm;
} LogEntry_t;

static LogEntry_t log_buffer[LOG_MAX_ENTRIES];
static int log_head  = 0;
static int log_count = 0;

static volatile bool cleanup_requested = false;

/* 給 main.c 的 Timer callback 呼叫 */
void task_logger_trigger_cleanup(void) {
    cleanup_requested = true;
}

static void log_write(float t, float h, uint32_t ts, bool alarm) {
    log_buffer[log_head].temperature  = t;
    log_buffer[log_head].humidity     = h;
    log_buffer[log_head].timestamp_ms = ts;
    log_buffer[log_head].is_alarm     = alarm;

    log_head = (log_head + 1) % LOG_MAX_ENTRIES;
    if (log_count < LOG_MAX_ENTRIES) log_count++;
}

static void log_dump(void) {
    printf("\n====== Log Dump (%d 筆) ======\n", log_count);
    int start = (log_count < LOG_MAX_ENTRIES) ? 0 : log_head;
    for (int i = 0; i < log_count; i++) {
        int idx = (start + i) % LOG_MAX_ENTRIES;
        printf("[%6lu ms] T=%.1f°C H=%.1f%% %s\n",
               (unsigned long)log_buffer[idx].timestamp_ms,
               log_buffer[idx].temperature,
               log_buffer[idx].humidity,
               log_buffer[idx].is_alarm ? "⚠ ALARM" : "");
    }
    printf("==============================\n\n");
}

void vTask_Logger(void *pvParameters) {
    (void)pvParameters;

    printf("[Logger] 啟動，等待資料...\n");

    while (1) {
        /* 1. 等警報 Semaphore（最多 500ms） */
        if (xSemaphoreTake(xAlarmSemaphore, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                log_write(gSharedData.temperature, gSharedData.humidity,
                          gSharedData.timestamp_ms, true);
                printf("[Logger] 警報事件已記錄！T=%.1f°C\n", gSharedData.temperature);
                xSemaphoreGive(xDataMutex);
            }
        }

        /* 2. 週期性記錄正常資料（直接讀 gSharedData，不和 Alarm 搶 Queue） */
        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            SensorData_t snapshot = gSharedData;
            xSemaphoreGive(xDataMutex);
            if (snapshot.valid) {
                log_write(snapshot.temperature, snapshot.humidity,
                          snapshot.timestamp_ms, false);
            }
        }

        /* 3. 處理 Software Timer 觸發的清理 */
        if (cleanup_requested) {
            cleanup_requested = false;
            printf("[Logger] Software Timer 觸發：清理舊紀錄\n");
            log_dump();
            log_head  = 0;
            log_count = 0;
            printf("[Logger] 清理完成\n");
        }
    }
}
