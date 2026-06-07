#include "SHT30.h"
#include "MYIIC.h"

/**
 * @brief  SHT30初始化
 * @note   主要是初始化IIC引脚并复位传感器
 * @retval 0:成功; 1:失败
 */
uint8_t SHT30_Init(void)
{
    IIC_Init(); // 初始化模拟IIC引脚
    return 0;
}

/**
 * @brief  CRC8 校验函数
 * @note   SHT30 内部自带 CRC 校验，用来确保读取的数据没有受硬件干扰报错
 */
static uint8_t SHT30_Check_CRC(uint8_t *data, uint8_t nbytes, uint8_t checksum)
{
    uint8_t crc = 0xFF;
    for (uint8_t byteIndex = 0; byteIndex < nbytes; byteIndex++)
    {
        crc ^= data[byteIndex];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return (crc == checksum) ? 0 : 1; // 0代表校验通过
}

/**
 * @brief  读取 SHT30 的温湿度数据
 * @param  temperature: 转换后的温度结果（摄氏度）
 * @param  humidity   : 转换后的湿度结果（%RH）
 * @retval 0:读取成功; 其他值:读取失败
 */
uint8_t SHT30_Read_Data(float *temperature, float *humidity)
{
    uint8_t buf[6];
    uint16_t raw_temp, raw_hum;

    // ---- 1. 发送测量指令 0x2400 (高连续性、无时钟拉伸) ----
    IIC_Start();
    IIC_Send_Byte((SHT30_ADDR << 1) | 0); // 准备写入
    if(IIC_Wait_Ack()) { IIC_Stop(); return 1; }

    IIC_Send_Byte(0x24); // 测量命令高字节
    if(IIC_Wait_Ack()) { IIC_Stop(); return 2; }
    IIC_Delay(); // 发送命令后稍微延时一下，确保SHT30有时间处理命令
    IIC_Send_Byte(0x00); // 测量命令低字节
    if(IIC_Wait_Ack()) { IIC_Stop(); return 3; }
    IIC_Stop();

    // ---- 2. 等待 SHT30 芯片内部测量完成 ----
    // SHT30在高精度测量下通常需要 15ms 左右的时间，这里延时 20ms 最稳妥
    HAL_Delay(20);

    // ---- 3. 读取测量出的 6 个字节数据 ----
    IIC_Start();
    IIC_Send_Byte((SHT30_ADDR << 1) | 1); // 准备读取
    if(IIC_Wait_Ack()) { IIC_Stop(); return 4; }

    buf[0] = IIC_Read_Byte(1); // 温度高8位，发 ACK
    buf[1] = IIC_Read_Byte(1); // 温度低8位，发 ACK
    buf[2] = IIC_Read_Byte(1); // 温度CRC校验位，发 ACK
    buf[3] = IIC_Read_Byte(1); // 湿度高8位，发 ACK
    buf[4] = IIC_Read_Byte(1); // 湿度低8位，发 ACK
    buf[5] = IIC_Read_Byte(0); // 湿度CRC校验位，最后一位发 NACK 结束
    IIC_Stop();

    // ---- 4. CRC 校验数据是否损坏 (可选，为了代码健壮性加上) ----
    if(SHT30_Check_CRC(&buf[0], 2, buf[2]) != 0) return 5; // 温度CRC错误
    if(SHT30_Check_CRC(&buf[3], 2, buf[5]) != 0) return 6; // 湿度CRC错误

    // ---- 5. 根据数据手册公式转换成真实物理值 ----
    raw_temp = ((uint16_t)buf[0] << 8) | buf[1];
    raw_hum  = ((uint16_t)buf[3] << 8) | buf[4];

    // 温度转换公式：T = -45 + 175 * (raw / (2^16 - 1))
    *temperature = -45.0f + 175.0f * (float)raw_temp / 65535.0f;
    // 湿度转换公式：RH = 100 * (raw / (2^16 - 1))
    *humidity = 100.0f * (float)raw_hum / 65535.0f;

    return 0; // 读取并转换成功
}
