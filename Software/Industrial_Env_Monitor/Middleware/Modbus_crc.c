#include "Modbus_crc.h"
/**
 * @brief  计算 Modbus CRC16 校验码
 * @param  *puchMsg: 要计算的数据缓冲区指针 (通常是数组名)
 * @param  usDataLen: 参与计算的数据长度 (字节数)
 * @retval 16位的 CRC 校验码
 */
uint16_t Modbus_CRC16(uint8_t *puchMsg, uint16_t usDataLen)
{
    uint16_t crc = 0xFFFF; // 初始值
    uint16_t i, j;

    for (i = 0; i < usDataLen; i++)
    {
        crc ^= puchMsg[i]; // 将当前字节与 CRC 的低 8 位进行异或
        
        for (j = 0; j < 8; j++) // 每个字节有 8 位，循环 8 次
        {
            // 检查最低位是否为 1
            if (crc & 0x0001) 
            {
                crc >>= 1;     // 如果为 1，先右移一位
                crc ^= 0xA001; // 然后与多项式 0xA001 异或
            }
            else 
            {
                crc >>= 1;     // 如果为 0，直接右移一位
            }
        }
    }
    return crc;
}
