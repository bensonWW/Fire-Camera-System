#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// ====================================================================================
// == 1. 基礎核心設定 ==
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      150000000 /* RP2350 預設系統時脈 150MHz（133 是 RP2040 的值） */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 512 ) // 對齊官方範例（SMP idle/timer 需要較大堆疊）
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     1  // lwip 的 FreeRTOS sys_arch 編譯需要
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

// 🎯【Cortex-M33 (RP2350) 硬體晶片規格設定】
#define configENABLE_FPU                        1  // Pico 2 W 有硬體 FPU，開啟它
#define configENABLE_MPU                        0  // 暫不使用記憶體保護
#define configENABLE_TRUSTZONE                  0  // 非 TrustZone 移植層
// ⚠️【關鍵】RP2350 必設：Pico SDK 全程跑在 secure world。沒設這行時 FreeRTOS 預設 0，
// port 會以為要做 secure/non-secure 世界切換，context/SVC/堆疊設定全錯 → 開機就 hard
// fault → USB 死。port.c 文件明載：SDK 環境必須 SECURE_ONLY=1 且 TRUSTZONE=0。
#define configRUN_FREERTOS_SECURE_ONLY          1

// ====================================================================================
// == 2. 多核心 (SMP) 調度設定 —— 對齊 Raspberry Pi 官方範例（雙核心 SMP）==
#define configNUMBER_OF_CORES                   2
#define configTICK_CORE                         0
#define configRUN_MULTIPLE_PRIORITIES           1
#define configUSE_CORE_AFFINITY                 1
#define configUSE_PASSIVE_IDLE_HOOK             0

// ====================================================================================
// == 3. 記憶體管理設定 ==
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 128 * 1024 ) ) // 128KB Heap 空間
#define configAPPLICATION_ALLOCATED_HEAP        0

// ====================================================================================
// == 4. 鉤子函式 (Hooks) 設定 ==
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2  // 開啟嚴格堆疊溢位檢查
#define configUSE_MALLOC_FAILED_HOOK            1  // 開啟動態記憶體失敗檢查
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

// ====================================================================================
// == 5. 執行狀態與計時器設定 ==
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

// 軟體計時器 (Software Timers)
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            1024

// ====================================================================================
// == 6. 斷言 (Assert) 偵錯設定 ==
// ⚠️【關鍵】這份 FreeRTOSConfig.h 會被強制 include 進整個 Pico SDK 的編譯。
// 原本自訂的 configASSERT 會 portDISABLE_INTERRUPTS()+for(;;)，一旦在 USB 起來前
// （例如 stdio_init_all 內 malloc 用到鎖時）觸發，就永久關中斷、USB 列舉失敗，
// 導致電腦「無法辨識」。改用官方範例的 assert()：Release build(NDEBUG) 下是 no-op，
// 不會關中斷。需要嚴格偵錯時改用 Debug build 即可。
#include <assert.h>
#define configASSERT( x ) assert( x )

// ====================================================================================
// == 7. 包含的 API 函式控制 ==
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitsFromISR       1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1

// ====================================================================================
// == 8. 中斷優先權與相容性補強 ==
#define portTICK_RATE_MS                        portTICK_PERIOD_MS
#define INCLUDE_xSemaphoreGetMutexHolder        1

// 中斷優先權：對齊官方範例。configKERNEL_INTERRUPT_PRIORITY 交給 port 預設處理，
// 只設 RP2350 專用的 configMAX_SYSCALL_INTERRUPT_PRIORITY=16（官方範例值）。
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     16

// ====================================================================================
// 註：portCHECK_IF_IN_ISR 與 SVC/PendSV/SysTick 向量掛載，皆由 Pico SDK 專用的
// RP2350 port（Community-Supported-Ports/GCC/RP2350_ARM_NTZ）自行提供，
// 不可在此 include "portable.h" 手動補（會把 hardware/sync.h 在 __force_inline
// 定義前拉進來，造成 SDK 標頭整片解析錯誤）。

#endif /* FREERTOS_CONFIG_H */