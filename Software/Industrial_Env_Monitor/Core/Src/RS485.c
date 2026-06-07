#include "RS485.h"
#include "stm32f1xx_hal.h"

// UART句柄
static UART_HandleTypeDef *RS485_UART = NULL;

// DE/RE控制
static GPIO_TypeDef *RS485_DE_RE_Port = NULL;
static uint16_t RS485_DE_RE_Pin = 0;

// DMA接收缓冲区
#define RS485_RX_BUF_SIZE 256
static uint8_t rs485_rx_buf[RS485_RX_BUF_SIZE];

// -------------------- DE/RE控制 --------------------
static void RS485_TxEnable(void)
{
    HAL_GPIO_WritePin(RS485_DE_RE_Port, RS485_DE_RE_Pin, GPIO_PIN_SET);
}

static void RS485_RxEnable(void)
{
    HAL_GPIO_WritePin(RS485_DE_RE_Port, RS485_DE_RE_Pin, GPIO_PIN_RESET);
}

// -------------------- 初始化 --------------------
void RS485_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *DE_RE_Port, uint16_t DE_RE_Pin)
{
    RS485_UART = huart;
    RS485_DE_RE_Port = DE_RE_Port;
    RS485_DE_RE_Pin = DE_RE_Pin;

    RS485_RxEnable();

    HAL_UARTEx_ReceiveToIdle_DMA(RS485_UART, rs485_rx_buf, RS485_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(RS485_UART->hdmarx, DMA_IT_HT);
}

// -------------------- 启动接收 --------------------
void RS485_ReceiveStart(void)
{
    // 先中止可能正在进行的DMA，确保状态干净
    HAL_UART_DMAStop(RS485_UART);
    RS485_RxEnable();
    HAL_UARTEx_ReceiveToIdle_DMA(RS485_UART, rs485_rx_buf, RS485_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(RS485_UART->hdmarx, DMA_IT_HT);
}

// -------------------- 发送 --------------------
void RS485_Send(uint8_t *buf, uint16_t len)
{
    // 先中止正在进行的RX DMA，再切换到发送
    HAL_UART_DMAStop(RS485_UART);
    RS485_TxEnable();
    HAL_UART_Transmit_DMA(RS485_UART, buf, len);
    // 发送完成后自动回调 HAL_UART_TxCpltCallback 切回接收
}

// -------------------- DMA+IDLE回调 --------------------
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == RS485_UART)
    {
        // 用户自定义回调处理接收到的数据
        RS485_RxCpltCallback(rs485_rx_buf, Size);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        RS485_Send(rs485_rx_buf, Size); // 回显收到的数据
        // 注意：这里不重启RX DMA，等TX完成后在TxCpltCallback中重启
    }
}

// -------------------- DMA发送完成回调 --------------------
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == RS485_UART)
    {
        // 发送完成后，切回接收模式并重启RX DMA
        RS485_ReceiveStart();
    }
}
void RS485_RxCpltCallback(uint8_t *buf, uint16_t len)
{
    // 用户实现的接收完成回调函数
    // 这里可以处理接收到的数据，例如解析命令、存储数据等
    // 目前示例中直接回显收到的数据，实际应用中请根据需要修改
}