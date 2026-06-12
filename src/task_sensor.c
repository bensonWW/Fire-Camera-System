/* ============================================================
 * task_sensor.c — 每 1 秒讀取 DHT11（改用 vmilea/pico_dht 的 PIO 驅動）
 * PIO 用硬體狀態機處理時序，不需關中斷，雙核 RTOS 下穩定可靠。
 * ============================================================ */

#include "shared.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pico/stdlib.h"
#include <dht.h>
#include <stdio.h>

static dht_t s_dht;

void vTask_Sensor(void *pvParameters) {
    (void)pvParameters;

    /* 初始化：DHT22/AM2302 機型、用 pio1（避開 cyw43 的 pio0）、開內部上拉 */
    dht_init(&s_dht, DHT22, pio1, GPIO_DHT22, true);

    /* MQ-2 煙霧感測器 DO（數位門檻輸出） */
    gpio_init(GPIO_MQ2_DO);
    gpio_set_dir(GPIO_MQ2_DO, GPIO_IN);

    vTaskDelay(pdMS_TO_TICKS(2000));   /* 上電穩定（MQ-2 加熱需要更久才準，這裡先讓系統跑） */

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);   /* 1 秒週期 */

    SensorData_t data;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);   /* 精確週期 */

        dht_start_measurement(&s_dht);
        float humidity = 0, temperature_c = 0;
        dht_result_t r = dht_finish_measurement_blocking(&s_dht, &humidity, &temperature_c);

        /* 讀 MQ-2 煙霧（DO 數位）—— 不論溫濕度成功與否都讀，煙霧獨立 */
        data.smoke = (gpio_get(GPIO_MQ2_DO) == MQ2_SMOKE_ACTIVE);

        data.timestamp_ms = xTaskGetTickCount();
        data.valid = (r == DHT_RESULT_OK);

        if (data.valid) {
            data.temperature = temperature_c;
            data.humidity    = humidity;
            printf("[Sensor] %.1f°C  %.1f%%RH  煙霧:%s\n",
                   temperature_c, humidity, data.smoke ? "⚠有" : "無");
        } else if (r == DHT_RESULT_TIMEOUT) {
            printf("[Sensor] 溫濕度讀取失敗：感測器無回應  煙霧:%s\n", data.smoke ? "⚠有" : "無");
        } else {
            printf("[Sensor] 溫濕度讀取失敗：校驗和錯誤  煙霧:%s\n", data.smoke ? "⚠有" : "無");
        }

        /* 🔍 煙霧校正診斷：乾淨空氣時看此值，MQ2_SMOKE_ACTIVE 要設成「相反」的值 */
        printf("[DO] 煙霧GP%d=%d\n", GPIO_MQ2_DO, gpio_get(GPIO_MQ2_DO));

        /* 一律送進 Queue + 更新共享資料（讓煙霧即使溫濕度失敗也能觸發火災） */
        if (xQueueSend(xSensorQueue, &data, 0) != pdTRUE) {
            SensorData_t discard;
            xQueueReceive(xSensorQueue, &discard, 0);
            xQueueSend(xSensorQueue, &data, 0);
        }
        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            gSharedData = data;
            xSemaphoreGive(xDataMutex);
        }
    }
}
