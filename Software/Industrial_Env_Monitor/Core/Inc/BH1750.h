#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"

/* * BH1750 的 I2C 地址
 * 如果模块的 ADDR 引脚接 GND（最常见），地址是 0x23
 * 如果模块的 ADDR 引脚接 3.3V，地址是 0x5C
 */
#define BH1750_ADDR 0x23 

/* 函数声明 */
void BH1750_Init(void);
float BH1750_Read_Light(void);

#endif