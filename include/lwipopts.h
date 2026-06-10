#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// 🎯【終極修正】：強迫 LwIP 切換為完全不抢鎖的裸機非同步模式，徹底消滅編譯與執行衝突
#define NO_SYS                      1

#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_SERVER_ONLY            1

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

#endif /* _LWIPOPTS_H */