#ifndef MODBUS_H
#define MODBUS_H

#include "main.h"
extern uint16_t Modbus_Reg[10];
extern uint8_t Rx_Buffer[30];
extern uint8_t Rx_Len;
extern uint8_t Save_Flag;
extern uint16_t address_06;
#define Slave_Address 0x01 // 从机地址

uint16_t Modbus_CRC16(uint8_t *puchMsg, uint16_t usDataLen);
uint8_t Modbus_Slave_Process(void);
#endif /* MODBUS_H */
