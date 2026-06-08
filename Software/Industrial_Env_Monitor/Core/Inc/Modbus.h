#ifndef MODBUS_H
#define MODBUS_H

#include "main.h"
#define Slave_Address 0x01 // 从机地址

uint16_t Modbus_CRC16(uint8_t *puchMsg, uint16_t usDataLen);
void Modbus_Slave_Process(void);
#endif /* MODBUS_H */
