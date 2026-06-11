/* ============================================================
 * shared.h — 跨 Task 共享的資料結構、GPIO 定義與全域物件宣告
 * （整合進 Edge-FireCam 專案的溫度警報子系統）
 * ============================================================ */
#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* ── GPIO 腳位定義 ── */
#define GPIO_DHT22      0    /* DHT22 DATA 腳 = GP0（實體第 1 腳） */
#define GPIO_MQ2_DO     16   /* MQ-2 煙霧 DO 數位輸出 = GP16（實體第 21 腳） */
#define GPIO_KY037_AO   26   /* KY-037 聲音 AO 類比輸出 = GP26 (ADC0，實體第 31 腳) */
#define KY037_ADC_INPUT 0    /* GP26 = ADC0 */
#define GPIO_LED_GREEN  14   /* 綠燈：正常（串 220~330Ω 限流電阻） */
#define GPIO_LED_RED    15   /* 紅燈：警報（串 220~330Ω） */

/* MQ-2 偵測到煙時 DO 的電位（若行為相反改 1）：看模組觸發 LED 亮時 DO 是高或低 */
#define MQ2_SMOKE_ACTIVE    0

/* 聲音偵測（抗雜訊）：算「偏離平均值夠多的樣本數」，避免單一 ADC 毛刺誤判。
 *  - SOUND_DEV：樣本偏離平均超過此值(ADC count)才算一個「響樣本」
 *  - SOUND_LOUD_COUNT：800 取樣中要有這麼多「響樣本」才判定有聲音（越小越靈敏） */
#define SOUND_DEV          40
#define SOUND_LOUD_COUNT   20

/* ── 溫度閾值（可調整） ── */
#define TEMP_ALARM_HIGH  35.0f   /* 超過此溫度 → 警報 */
#define TEMP_ALARM_LOW    5.0f   /* 低於此溫度 → 警報 */

/* ── 感測器資料結構 ── */
typedef struct {
    float    temperature;    /* 攝氏溫度 */
    float    humidity;       /* 相對濕度 % */
    bool     smoke;          /* MQ-2 是否偵測到煙 */
    uint32_t timestamp_ms;   /* xTaskGetTickCount 時間戳 */
    bool     valid;          /* 溫濕度資料是否有效 */
} SensorData_t;

/* ── 聲音事件旗標（vTask_Sound 設定，WebServer 讀取顯示） ── */
extern volatile bool g_sound_alert;

/* ── 全域 Kernel 物件（main.c 定義，其他檔案 extern） ── */
extern QueueHandle_t      xSensorQueue;    /* Sensor → Alarm */
extern SemaphoreHandle_t  xAlarmSemaphore; /* Alarm → Logger 警報通知 */
extern SemaphoreHandle_t  xDataMutex;      /* 保護 gSharedData */
extern SensorData_t       gSharedData;     /* 最新一筆感測資料（供 WebServer 讀） */

/* ── Task 進入點 ── */
void vTask_Sensor(void *pvParameters);
void vTask_Alarm(void *pvParameters);
void vTask_Logger(void *pvParameters);
void vTask_Sound(void *pvParameters);

/* ── Logger 清理觸發（Software Timer callback 呼叫） ── */
void task_logger_trigger_cleanup(void);

#endif /* SHARED_H */
