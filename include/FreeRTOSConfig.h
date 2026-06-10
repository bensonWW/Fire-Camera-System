#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// ====================================================================================
// == 1. 基礎核心設定 ==
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      133000000
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 256 ) // 🎯 提高至 256 Word，確保中斷與任務切換安全
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
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

// 🎯【Cortex-M33 (RP2350) 硬體晶片規格設定】
#define configENABLE_FPU                        1  // Pico 2 W 有硬體 FPU，開啟它
#define configENABLE_MPU                        0  // 暫不使用記憶體保護
#define configENABLE_TRUSTZONE                  0  // 選用 Non-TrustZone 移植層，強設為 0

// ====================================================================================
// == 2. 多核心調度設定（單核心模式運作） ==
#define configNUMBER_OF_CORES                   1 
#define configRUN_MULTIPLE_PRIORITIES           0
#define configUSE_PASSIVE_LISTS                 0
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
#define configASSERT( x ) if( ( x ) == 0 ) { portDISABLE_INTERRUPTS(); for( ;; ); }

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

// 🎯【🚨 核心修正死鎖關鍵 🚨】
// 移除原本手動做的 `<< 5` 位移，直接填入硬體優先權數值（Cortex-M33 數值範圍 0~7）。
// 7 代表最低優先權，5 代表 SysTick 系統呼叫保護邊界。
#define configKERNEL_INTERRUPT_PRIORITY          7 /* 讓作業系統心跳維持在硬體最低優先權 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     5 /* 高於此優先權的中斷不受 RTOS 影響 */

// ====================================================================================
// 🎯【Pico SDK 2.2.0 網路庫編譯修復】
// 修正 async_context_freertos.c 找不到 portCHECK_IF_IN_ISR 的官方 Bug。
#ifndef portCHECK_IF_IN_ISR
    #include "projdefs.h" 
    #include "portable.h"

    static inline __attribute__((always_inline)) BaseType_t xPortCheckIfInISR(void) {
        uint32_t ulCurrentInterrupt;
        // 讀取 Cortex-M33 的 IPSR 暫存器：若值大於 0 則代表目前處於中斷 (ISR) 狀態
        __asm volatile ("mrs %0, ipsr" : "=r" (ulCurrentInterrupt) :: "memory");
        return (ulCurrentInterrupt > 0) ? pdTRUE : pdFALSE;
    }
    #define portCHECK_IF_IN_ISR()    xPortCheckIfInISR()
#endif

// 🎯【中斷向量表對齊】將 FreeRTOS 的中斷處理函式直接映射至 Pico SDK 硬體中斷名稱
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */