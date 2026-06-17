#ifndef __RS485_H
#define __RS485_H

#include "main.h"
#include <stdint.h>
#include <stddef.h>

// 初始化RS485驱动，必须传入已经初始化的UART句柄
void RS485_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *DE_RE_Port, uint16_t DE_RE_Pin);

// 发送一帧数据（非阻塞，使用DMA）
void RS485_Send(uint8_t *buf, uint16_t len);

// 开启接收（用于手动重启DMA接收）
void RS485_ReceiveStart(void);

// 处理接收到的数据的回调函数（用户实现）
void RS485_RxCpltCallback(uint8_t *buf, uint16_t len);

#endif
