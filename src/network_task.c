#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "network_task.h"

#define WIFI_SSID "bensonW"       // 🎯 請替換成你的 Wi-Fi 名稱
#define WIFI_PASSWORD "benson25" // 🎯 請替換成你的 Wi-Fi 密碼

void network_task(void *pvParameters) {
    printf("[Network] 網路任務已安全喚醒...\n");
    fflush(stdout);
    // ❌ 刪除或註解掉原本任務內部的 cyw43_arch_init() 與 cyw43_arch_enable_sta_mode()

    printf("[Network] 正在連線至 Wi-Fi: %s ...\n", WIFI_SSID);

    // 專心連線 Wi-Fi
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("[Network] ❌ 錯誤: Wi-Fi 連線失敗！\n");
        vTaskDelete(NULL);
    }

    printf("[Network] 🎉 Wi-Fi 連線成功！\n");
    
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