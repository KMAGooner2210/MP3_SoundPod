#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* --- C?u hình co b?n cho STM32F103 --- */
#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( ( unsigned long ) 72000000 ) /* 72MHz */
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )        /* 1ms tick */
#define configMAX_PRIORITIES            ( 5 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 10 * 1024 ) )   /* 10KB RAM cho Heap */
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1

/* --- Software Timer (T?t d? g?n nh?) --- */
#define configUSE_TIMERS                0

/* --- API Functions --- */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1

/* --- Cortex-M3 Priority Config (R?t quan tr?ng) --- */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS       __NVIC_PRIO_BITS
#else
#define configPRIO_BITS       4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xf
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* --- Mapping Interrupts (Chìa khóa d? ch?y trên Keil RTE) --- */
/* FreeRTOS s? t? d?ng chi?m quy?n di?u khi?n 3 ng?t này */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
