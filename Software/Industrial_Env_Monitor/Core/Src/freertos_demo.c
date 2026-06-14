#include "freertos_demo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Modbus.h"
#include "Modbus_register.h"
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
void Sensor_Task(void * pvParameters);

/*任务2的配置*/
#define TASK2_SATCK 128
#define TASK2_PRIORITY 3
TaskHandle_t task2_handle;
StackType_t task2_stack[TASK2_SATCK];
StaticTask_t task2_tcb;
void Modbus_Task(void * pvParameters);

/*任务3的配置*/
#define TASK3_SATCK 128
#define TASK3_PRIORITY 4
TaskHandle_t task3_handle;
StackType_t task3_stack[TASK3_SATCK];
StaticTask_t task3_tcb;
void System_Task(void * pvParameters);

/*======静态创建方式，需要手动创建两个特殊任务的资源======*/
/*分配空闲任务*/
StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
StaticTask_t idle_task_tcb;
/*消息队列*/
QueueHandle_t modbusCommandQueue = NULL;
static uint8_t modbusCommandQueueBuf[16 * sizeof(ModbusCommand_t)];
static StaticQueue_t modbusCommandQueueStruct;

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
    //创建消息队列
    modbusCommandQueue = xQueueCreateStatic(16, sizeof(ModbusCommand_t), modbusCommandQueueBuf, &modbusCommandQueueStruct);
    //使用静态创建三个任务
    taskENTER_CRITICAL();
    task1_handle = xTaskCreateStatic((TaskFunction_t) Sensor_Task,
                            (char *) "Sensor_Task", 
                            (uint32_t) TASK1_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK1_PRIORITY,
                            (StackType_t *) task1_stack,  //任务栈地址
                            (StaticTask_t *)&task1_tcb  //任务句柄地址
                        );
    task2_handle = xTaskCreateStatic((TaskFunction_t) Modbus_Task,
                            (char *) "Modbus_Task", 
                            (uint32_t) TASK2_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK2_PRIORITY,
                            (StackType_t *) task2_stack,  //任务栈地址
                            (StaticTask_t *)&task2_tcb  //任务句柄地址
                        );
    task3_handle = xTaskCreateStatic((TaskFunction_t) System_Task,
                            (char *) "System_Task", 
                            (uint32_t) TASK3_SATCK,
                            (void *) NULL,
                            (UBaseType_t) TASK3_PRIORITY,
                            (StackType_t *) task3_stack,  //任务栈地址
                            (StaticTask_t *)&task3_tcb  //任务句柄地址
                        );
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
}

void Sensor_Task(void * pvParameters)
{
    while(1)
    {
        // ================== 读取 SHT30 ==================
        if(SHT30_Read_Data(&temperature, &humidity) == 0)
        {
            //printf("温度: %.2f °C, 湿度: %.2f %%RH\r\n", temperature,humidity);
            __disable_irq();
            Modbus_Reg[REG_TEMP] = (uint16_t)(temperature * 10.0f);
            Modbus_Reg[REG_HUMI] = (uint16_t)(humidity * 10.0f);
            __enable_irq();
        }
        // // ================== 读取 BH1750 ==================
        light = BH1750_Read_Light();
        if(light >= 0) // 返回值大于等于0说明读取成功
        {
        // printf("光照: %.1f Lux\r\n", light);
            __disable_irq();
            Modbus_Reg[REG_LIGHT] = (uint16_t)(light * 10.0f);
            __enable_irq();
        }
        vTaskDelay(10);
    }

}
void Modbus_Task(void * pvParameters)
{
    ModbusCommand_t msg;
    while(1)
    {
        if(xQueueReceive(modbusCommandQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            // 处理 Modbus 命令
            if(msg.cmd == 0x06) // 写单个寄存器命令
            {
                Save_Params(address_06);
            }
        }
    }

}
void System_Task(void * pvParameters)
{
    while(1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // 翻转LED，观察程序运行状态
        vTaskDelay(100);
    }

}

