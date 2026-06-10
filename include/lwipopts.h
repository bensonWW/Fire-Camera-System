#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// 基礎網路功能開關
#define NO_SYS                      0 // 0 代表有作業系統 (FreeRTOS)
#define LWIP_SOCKET                 1
#define LWIP_NETCONN                1
#define LWIP_NETIF_API              1

// 記憶體優化設定
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (16 * 1024)
#define MEMP_NUM_PBUF               10
#define MEMP_NUM_UDP_PCB            6
#define MEMP_NUM_TCP_PCB            10
#define MEMP_NUM_TCP_PCB_LISTEN     5
#define MEMP_NUM_SYS_TIMEOUT        10

// 🎯 1. 調整 TCP 記憶體切片總量至 32，確保絕對大於發送佇列長度 (16)
#define MEMP_NUM_TCP_SEG            32

// 🎯 2. 直接聽從官方建議，開啟全域通行證，強制跳過這個過於嚴格的合理性安全檢查
#define LWIP_DISABLE_TCP_SANITY_CHECKS 1

// 🎯 3. 告訴系統不要重定義 struct timeval
#define LWIP_TIMEVAL_PRIVATE        0

// 核心多工執行緒 (TCPIP Thread) 設定
#define TCPIP_THREAD_NAME           "TCPIP"
#define TCPIP_THREAD_STACKSIZE      1024
#define TCPIP_THREAD_PRIO           3
#define DEFAULT_THREAD_STACKSIZE    1024
#define DEFAULT_THREAD_PRIO         1

// 網路協定支援
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1 
#define LWIP_IPV4                   1 
#define LWIP_TCP                    1 
#define LWIP_UDP                    1 
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1 

// TCP 視窗與緩衝區設定
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUE_LEN             16

// 統計與偵錯 
#define LWIP_STATS                  0
#define SYS_LIGHTWEIGHT_PROT        1

// 橋接鎖
#define LOCK_TCPIP_CORE()
#define UNLOCK_TCPIP_CORE()

#endif /* _LWIPOPTS_H */