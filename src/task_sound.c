/* ============================================================
 * task_sound.c — KY-037 聲音感測（用 AO 類比輸出 + ADC 量音量）
 * 快速取樣 AO，算「峰對峰振幅」(peak-to-peak)，超過門檻視為有聲音。
 * 比 DO 數位門檻可靠，且不依賴模組電位器。
 * ============================================================ */

#include "shared.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

#define SOUND_HOLD_MS   2000   /* 偵測到後旗標至少維持這麼久 */
#define SOUND_SAMPLES   800    /* 每次取樣數 */

volatile bool g_sound_alert = false;

static uint16_t s_buf[SOUND_SAMPLES];   /* 取樣緩衝（static，不吃 task 堆疊） */

void vTask_Sound(void *pvParameters) {
    (void)pvParameters;

    adc_init();
    adc_gpio_init(GPIO_KY037_AO);

    TickType_t lastDetect = 0;
    uint32_t   printDiv   = 0;

    while (1) {
        /* 取樣 + 求平均 */
        adc_select_input(KY037_ADC_INPUT);
        uint32_t sum = 0;
        for (int i = 0; i < SOUND_SAMPLES; i++) {
            s_buf[i] = adc_read();
            sum += s_buf[i];
        }
        int avg = (int)(sum / SOUND_SAMPLES);

        /* 數「偏離平均夠多」的樣本（抗單一毛刺）；順便算 pp 供顯示 */
        int loud = 0;
        uint16_t mn = 4095, mx = 0;
        for (int i = 0; i < SOUND_SAMPLES; i++) {
            int dev = (int)s_buf[i] - avg;
            if (dev < 0) dev = -dev;
            if (dev > SOUND_DEV) loud++;
            if (s_buf[i] < mn) mn = s_buf[i];
            if (s_buf[i] > mx) mx = s_buf[i];
        }
        bool active = (loud >= SOUND_LOUD_COUNT);

        if (active) {
            if (!g_sound_alert) {
                printf("[Sound] 🔊 偵測到異常聲響！(響樣本=%d)\n", loud);
            }
            g_sound_alert = true;
            lastDetect = xTaskGetTickCount();
        } else if (g_sound_alert &&
                   (xTaskGetTickCount() - lastDetect > pdMS_TO_TICKS(SOUND_HOLD_MS))) {
            g_sound_alert = false;
            printf("[Sound] 聲響解除\n");
        }

        /* 每約 1 秒印一次供校正：響樣本數、pp、avg */
        if (++printDiv >= 20) {
            printDiv = 0;
            printf("[Sound] 響樣本=%d  pp=%u  avg=%d（門檻 %d 樣本）\n",
                   loud, (unsigned)(mx - mn), avg, SOUND_LOUD_COUNT);
        }

        vTaskDelay(pdMS_TO_TICKS(50));   /* 20Hz */
    }
}
