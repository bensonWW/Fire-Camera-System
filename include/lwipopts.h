#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// 🎯【終極修正】：強迫 LwIP 切換為完全不抢鎖的裸機非同步模式，徹底消滅編譯與執行衝突
#define NO_SYS                      1

#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_SERVER_ONLY            1

// ⚠️【關鍵】啟用 DHCP，否則 join 上 AP 後永遠拿不到 IP（停在 "no ip"）。
// cyw43_arch_wifi_connect_* 會一直等到取得 IP 才算成功，沒 DHCP 就 timeout。
#define LWIP_IPV4                   1
#define LWIP_UDP                    1   // DHCP 走 UDP，必須開
#define LWIP_DHCP                   1
#define LWIP_DNS                    1
#define DHCP_DOES_ARP_CHECK         0   // 加速取得 IP
#define LWIP_DHCP_DOES_ACD_CHECK    0

// 記憶體優化（適合嵌入式環境）
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10

// 啟用網頁伺服器 HTTPD 的基礎通訊協定
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)

#define LWIP_HTTPD                  1

// 🎯 啟用自訂檔案：用 fs_open_custom() 提供自己的網頁，覆蓋 lwIP 內建示範頁。
#define LWIP_HTTPD_CUSTOM_FILES     1
// 自訂檔案只給 HTML 內容（不含 HTTP header），讓 httpd 依副檔名自動產生 header。
#define LWIP_HTTPD_DYNAMIC_HEADERS  1

#endif /* _LWIPOPTS_H */