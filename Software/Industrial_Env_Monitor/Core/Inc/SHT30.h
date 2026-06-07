#ifndef __SHT30_H
#define __SHT30_H

#include "main.h"

/* * SHT30 的预设 I2C 地址
 * 如果你的 SHT30 模块上的 ADDR 引脚接了 GND（绝大多数模块的默认情况），地址是 0x44
 * 如果 ADDR 引脚接了 3.3V，地址需要改为 0x45
 */
#define SHT30_ADDR      0x44 

/* 函数声明 */
uint8_t SHT30_Init(void);
uint8_t SHT30_Read_Data(float *temperature, float *humidity);

#endif