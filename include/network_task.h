#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#include <stdbool.h>

// 共享狀態旗標（給 led_task 讀，顯示連線狀態）
extern volatile bool g_cyw43_ready;     // cyw43 晶片是否已初始化完成
extern volatile bool g_wifi_connected;  // Wi-Fi 是否已連線並取得 IP

void network_task(void *pvParameters);
void led_task(void *pvParameters);      // 板載 LED 狀態指示燈

#endif // NETWORK_TASK_H