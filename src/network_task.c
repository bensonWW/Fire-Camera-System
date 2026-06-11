#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "network_task.h"

#define WIFI_SSID "bensonW"       // 🎯 請替換成你的 Wi-Fi 名稱
#define WIFI_PASSWORD "benson25" // 🎯 請替換成你的 Wi-Fi 密碼

// 共享狀態旗標（led_task 讀取來決定閃爍模式）
volatile bool g_cyw43_ready = false;
volatile bool g_wifi_connected = false;

// 🎯 板載 LED 狀態指示燈（Pico 2 W 的 LED 接在 cyw43 晶片上，需用 cyw43_arch_gpio_put）
//   - cyw43 尚未就緒：LED 滅
//   - 連線中/未連上：快閃（150ms 開/關）
//   - 已連線：慢心跳（每 2 秒亮 80ms）
void led_task(void *pvParameters) {
    // 等 cyw43 初始化完成才能碰 LED（否則會 crash）
    while (!g_cyw43_ready) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    while (true) {
        if (g_wifi_connected) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(80));
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(1920));
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(150));
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(150));
        }
    }
}

void network_task(void *pvParameters) {
    // 等序列監視器連上再開始（最多 8 秒），避免開機 banner 與連線訊息被丟掉
    for (int i = 0; i < 80 && !stdio_usb_connected(); i++) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    // banner 移到這裡印（排程器已啟動，USB/stdio 可安全使用）
    printf("\n============================================\n");
    printf("   🚨 Edge-FireCam 消防相機系統 啟動中... 🚨   \n");
    printf("   執行平台: Raspberry Pi Pico 2 W (FreeRTOS)  \n");
    printf("============================================\n");
    printf("[Network] 網路任務已安全喚醒...\n");
    fflush(stdout);

    // 🎯【cyw43 初始化必須在排程器啟動後、於任務內執行】
    // FreeRTOS port 下 cyw43/lwip 的 async_context 依賴 FreeRTOS 同步機制，
    // 在 main() 裡（排程器啟動前）init 會 hard fault，連 USB 都會列舉失敗。
    printf("[Network] 正在初始化 cyw43 無線晶片硬體...\n");
    if (cyw43_arch_init()) {
        printf("[Network] ❌ 嚴重錯誤: cyw43 晶片硬體初始化失敗！任務結束。\n");
        vTaskDelete(NULL);
        return;
    }
    cyw43_arch_enable_sta_mode();
    g_cyw43_ready = true; // 通知 led_task 可以開始控制 LED 了
    printf("[Network] 🎉 cyw43 晶片硬體初始化成功！\n");

    // 🎯 連線重試迴圈：連不上不放棄，每次失敗等 5 秒再試，直到成功為止
    const uint32_t RETRY_DELAY_MS = 5000;
    uint32_t attempt = 0;
    while (true) {
        attempt++;
        printf("[Network] 正在連線至 Wi-Fi: %s ...（第 %lu 次嘗試）\n", WIFI_SSID, (unsigned long)attempt);

        int err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (err == 0) {
            break; // 連線成功，跳出重試迴圈
        }

        printf("[Network] ❌ Wi-Fi 連線失敗 (err=%d)，%lu 秒後重試...\n", err, (unsigned long)(RETRY_DELAY_MS / 1000));
        // 用 FreeRTOS 排程器禮讓 CPU 給其他任務，不要忙等
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
    }

    g_wifi_connected = true; // led_task 切換成「已連線」慢心跳
    printf("[Network] 🎉 Wi-Fi 連線成功！（共嘗試 %lu 次）\n", (unsigned long)attempt);
    
    struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
    printf("[Network] =============================================\n");
    printf("[Network] 💡 Pico 2 W 當前 IP 位址: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    printf("[Network] =============================================\n");

    printf("[Network] 正在啟動 HTTP 網頁伺服器...\n");
    httpd_init();
    printf("[Network] 🚀 Web Server 已在 Port 80 監聽！\n");

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}