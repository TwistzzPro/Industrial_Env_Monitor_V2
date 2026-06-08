#ifndef __MODBUS_CRC_H
#define __MODBUS_CRC_H

#include "main.h"

uint16_t Modbus_CRC16(uint8_t *puchMsg, uint16_t usDataLen);

#endif 
