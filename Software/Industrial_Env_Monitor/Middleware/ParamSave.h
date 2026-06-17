#ifndef __PARAM_SAVE_H
#define __PARAM_SAVE_H

#include "main.h"
#include "Modbus.h"
#include "Modbus_register.h"

void Load_Params(void);
void Save_Params(uint16_t Reg_Address);

#endif /* __PARAM_SAVE_H */
