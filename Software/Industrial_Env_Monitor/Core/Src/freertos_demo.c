#include "freertos_demo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/*启动任务的配置*/
#define START_TASK_SATCK 128
#define START_TASK_PRIORITY 1
TaskHandle_t stat_task_handle;
StackType_t start_task_stack[START_TASK_SATCK];
StaticTask_t satrt_task_tcb;
void stat_task(void * pvParameters);

/*任务1的配置*/
#define TASK1_SATCK 128
#define TASK1_PRIORITY 2
TaskHandle_t task1_handle;
StackType_t task1_stack[TASK1_SATCK];
StaticTask_t task1_tcb;
void task1(void * pvParameters);

/*任务2的配置*/
#define TASK2_SATCK 128
#define TASK2_PRIORITY 3
TaskHandle_t task2_handle;
StackType_t task2_stack[TASK2_SATCK];
StaticTask_t task2_tcb;
void task2(void * pvParameters);

/*任务3的配置*/
#define TASK3_SATCK 128
#define TASK3_PRIORITY 4
TaskHandle_t task3_handle;
StackType_t task3_stack[TASK3_SATCK];
StaticTask_t task3_tcb;
void task3(void * pvParameters);

/*======静态创建方式，需要手动创建两个特殊任务的资源======*/
/*分配空闲任务*/
StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
StaticTask_t idle_task_tcb;

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                        StackType_t ** ppxIdleTaskStackBuffer,
                                        uint32_t * pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &idle_task_tcb;
    *ppxIdleTaskStackBuffer = idle_task_stack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*分配定时器任务*/
StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];
StaticTask_t timer_task_tcb;
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                         StackType_t ** ppxTimerTaskStackBuffer,
                                         uint32_t * pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &timer_task_tcb;
    *ppxTimerTaskStackBuffer = timer_task_stack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void FreeRTOS_Init(void)
{
    //创建一个启动任务
    stat_task_handle = xTaskCreateStatic((TaskFunction_t) stat_task,
                            (char *) "stat_task", 
                            (uint32_t) START_TASK_SATCK,
                            (void *) NULL,
                            (UBaseType_t) START_TASK_PRIORITY,
                            (StackType_t *) start_task_stack,  //任务栈地址
                            (StaticTask_t *)&satrt_task_tcb  //任务句柄地址
                        );

    //启动调度器
    vTaskStartScheduler();

}
void stat_task(void * pvParameters)
{
    //使用静态创建三个任务
    taskENTER_CRITICAL();
    task1_handle = xTaskCreateStatic((TaskFunction_t) task1,
                            (char *) "task1", 
                            (uint32_t) TASK1_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK1_PRIORITY,
                            (StackType_t *) task1_stack,  //任务栈地址
                            (StaticTask_t *)&task1_tcb  //任务句柄地址
                        );
    task2_handle = xTaskCreateStatic((TaskFunction_t) task2,
                            (char *) "task2", 
                            (uint32_t) TASK2_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK2_PRIORITY,
                            (StackType_t *) task2_stack,  //任务栈地址
                            (StaticTask_t *)&task2_tcb  //任务句柄地址
                        );
    task3_handle = xTaskCreateStatic((TaskFunction_t) task3,
                            (char *) "task3", 
                            (uint32_t) TASK3_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK3_PRIORITY,
                            (StackType_t *) task3_stack,  //任务栈地址
                            (StaticTask_t *)&task3_tcb  //任务句柄地址
                        );
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
}

void task1(void * pvParameters)
{
    char message[] = "task1...\r\n";
    while(1)
    {
        HAL_UART_Transmit(&huart1,(uint8_t*)message,sizeof(message),1);
        HAL_GPIO_TogglePin(Y_LED_GPIO_Port,Y_LED_Pin);
        vTaskDelay(500);
    }

}
void task2(void * pvParameters)
{
    char message[] = "task2...\r\n";
    while(1)
    {
        HAL_UART_Transmit(&huart1,(uint8_t*)message,sizeof(message),1);
        HAL_GPIO_TogglePin(B_LED_GPIO_Port,B_LED_Pin);
        vTaskDelay(500);
    }

}
void task3(void * pvParameters)
{
    char message[] = "task3...\r\n";
    while(1)
    {
        HAL_UART_Transmit(&huart1,(uint8_t*)message,sizeof(message),1);
        vTaskDelay(500);
    }

}

