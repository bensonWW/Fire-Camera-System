/* ============================================================
 * task_alarm.c — 監聽 Queue，溫度超標立即觸發紅燈警報
 * Kernel API：xQueueReceive, xSemaphoreGive, GPIO
 * 優先權最高，確保警報即時響應
 * ============================================================ */

#include "shared.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "pico/stdlib.h"
#include <stdio.h>

#define ALARM_BLINK_COUNT  5
#define ALARM_BLINK_MS     200

typedef enum {
    ALARM_NONE = 0,
    ALARM_HIGH = 1,
    ALARM_LOW  = 2,
} AlarmState_t;

static AlarmState_t current_alarm = ALARM_NONE;

static void blink_red(int n) {
    for (int i = 0; i < n; i++) {
        gpio_put(GPIO_LED_RED, 1);
        vTaskDelay(pdMS_TO_TICKS(ALARM_BLINK_MS));
        gpio_put(GPIO_LED_RED, 0);
        vTaskDelay(pdMS_TO_TICKS(ALARM_BLINK_MS));
    }
}

void vTask_Alarm(void *pvParameters) {
    (void)pvParameters;

    gpio_init(GPIO_LED_GREEN);
    gpio_init(GPIO_LED_RED);
    gpio_set_dir(GPIO_LED_GREEN, GPIO_OUT);
    gpio_set_dir(GPIO_LED_RED,   GPIO_OUT);

    /* 開機亮綠燈，表示正常 */
    gpio_put(GPIO_LED_GREEN, 1);
    gpio_put(GPIO_LED_RED,   0);

    SensorData_t received;

    while (1) {
        if (xQueueReceive(xSensorQueue, &received, portMAX_DELAY) == pdTRUE) {
            float t = received.temperature;

            /* 火災判定：高溫 或 偵測到煙霧（任一即視為火災） */
            bool fire = (received.valid && t > TEMP_ALARM_HIGH) || received.smoke;
            bool cold = (received.valid && t < TEMP_ALARM_LOW);

            AlarmState_t new_alarm = fire ? ALARM_HIGH : (cold ? ALARM_LOW : ALARM_NONE);

            if (fire) {
                if (received.smoke && received.valid && t > TEMP_ALARM_HIGH)
                    printf("[Alarm] 🔥 火災！高溫 %.1f°C + 偵測到煙霧\n", t);
                else if (received.smoke)
                    printf("[Alarm] 🔥 火災！偵測到煙霧\n");
                else
                    printf("[Alarm] ⚠ 溫度過高！%.1f°C > %.1f°C\n", t, TEMP_ALARM_HIGH);
            } else if (cold) {
                printf("[Alarm] ⚠ 溫度過低！%.1f°C < %.1f°C\n", t, TEMP_ALARM_LOW);
            }

            if (new_alarm != ALARM_NONE) {
                gpio_put(GPIO_LED_GREEN, 0);   /* 綠燈滅 */
                blink_red(ALARM_BLINK_COUNT);  /* 紅燈閃 5 下 */
                gpio_put(GPIO_LED_RED, 1);     /* 紅燈常亮 */
                xSemaphoreGive(xAlarmSemaphore); /* 通知 Logger 記錄 */
            } else if (current_alarm != ALARM_NONE) {
                /* 警報解除（溫度回正常且無煙霧） */
                printf("[Alarm] ✓ 警報解除，恢復正常 %.1f°C\n", t);
                gpio_put(GPIO_LED_RED,   0);
                gpio_put(GPIO_LED_GREEN, 1);
            }

            current_alarm = new_alarm;
        }
    }
}
