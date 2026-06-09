#include "Modbus.h"
#include "Modbus_crc.h"
#include "RS485.h"

uint16_t Modbus_Reg[10];
uint8_t Rx_Buffer[30];
uint8_t Rx_Len = 0;
uint8_t Modbus_Slave_Process(void)
{
    if(Rx_Len == 0) return 0;
    uint16_t crc_calculated = Modbus_CRC16(Rx_Buffer, Rx_Len - 2); // 计算接收到的数据的 CRC16 校验码
    uint16_t crc_received = (Rx_Buffer[Rx_Len - 1] << 8) | Rx_Buffer[Rx_Len - 2]; // 从接收缓冲区提取 CRC16 校验码
    if(crc_calculated == crc_received)
    {
        // CRC 校验成功，处理数据
        if(Rx_Buffer[0] == Slave_Address) // 检查从机地址
        {
            if(Rx_Buffer[1] == 0x03) // 读保持寄存器功能码
            {
                uint16_t start_address = (Rx_Buffer[2] << 8) | Rx_Buffer[3];
                uint16_t register_count = (Rx_Buffer[4] << 8) | Rx_Buffer[5];
                if((start_address + register_count) <= 10) // 确保寄存器地址合法
                {
                    // 这里可以根据需要处理读寄存器的请求，例如将 Modbus_Reg 中的数据打包发送回主机
                    static uint8_t Tx_Buffer[30];
                    uint8_t Tx_Len = 0;
                    Tx_Buffer[Tx_Len++] = Slave_Address; // 从机地址
                    Tx_Buffer[Tx_Len++] = 0x03; // 功能码
                    Tx_Buffer[Tx_Len++] = register_count * 2; // 字节数
                    for(int i = 0; i<register_count; i++)
                    {
                        Tx_Buffer[Tx_Len++] = (Modbus_Reg[start_address + i] >> 8) & 0xFF; // 寄存器高字节
                        Tx_Buffer[Tx_Len++] = Modbus_Reg[start_address + i] & 0xFF; // 寄存器低字节
                    }
                    uint16_t crc_send = Modbus_CRC16(Tx_Buffer,Tx_Len);
                    Tx_Buffer[Tx_Len++] = crc_send & 0xFF; // CRC 低字节
                    Tx_Buffer[Tx_Len++] = (crc_send >> 8) & 0xFF; // CRC 高字节

                    RS485_Send(Tx_Buffer, Tx_Len); // 发送响应数据
                    return 1; 

                }
            }
        }
    }
    Rx_Len = 0; // 处理完成后清空接收长度，准备下一次接收
    return 0; // CRC 校验失败或其他错误

}
