#include "BH1750.h"
#include "MYIIC.h" // 直接引入你已经写好的软件 IIC 驱动

/**
 * @brief  BH1750 初始化
 * @note   发送通电指令
 */
void BH1750_Init(void)
{
    IIC_Start();
    IIC_Send_Byte((BH1750_ADDR << 1) | 0); // 发送写地址
    IIC_Wait_Ack();
    
    IIC_Send_Byte(0x01); // 0x01 是通电指令 (Power On)
    IIC_Wait_Ack();
    IIC_Stop();
}

/**
 * @brief  读取 BH1750 光照强度
 * @retval 返回光照度值 (单位: Lux)
 */
float BH1750_Read_Light(void)
{
    uint8_t buf[2];
    uint16_t raw_light;
    float lux;

    // ---- 1. 发送测量指令 ----
    // 0x20: 单次高分辨率模式 (One-Time H-Resolution Mode)
    // 分辨率 1 Lux，测量完自动进入省电模式，非常适合轮询读取
    IIC_Start();
    IIC_Send_Byte((BH1750_ADDR << 1) | 0); // 写操作
    if(IIC_Wait_Ack()) { IIC_Stop(); return -1.0f; } // 如果没连上，返回负数报错

    IIC_Send_Byte(0x20); // 发送单次测量命令
    if(IIC_Wait_Ack()) { IIC_Stop(); return -2.0f; }
    IIC_Stop();

    // ---- 2. 等待测量完成 ----
    // 数据手册规定高分辨率模式最大需要 180ms，我们延时 150~180ms 即可
    HAL_Delay(150);

    // ---- 3. 读取 2 个字节的光照数据 ----
    IIC_Start();
    IIC_Send_Byte((BH1750_ADDR << 1) | 1); // 读操作
    if(IIC_Wait_Ack()) { IIC_Stop(); return -3.0f; }

    buf[0] = IIC_Read_Byte(1); // 读高 8 位，发送 ACK
    buf[1] = IIC_Read_Byte(0); // 读低 8 位，发送 NACK (告诉传感器读完了)
    IIC_Stop();

    // ---- 4. 数据转换 ----
    raw_light = ((uint16_t)buf[0] << 8) | buf[1];
    
    // 根据数据手册，光照强度计算公式为：测得的数值 / 1.2
    lux = (float)raw_light / 1.2f;

    return lux;
}