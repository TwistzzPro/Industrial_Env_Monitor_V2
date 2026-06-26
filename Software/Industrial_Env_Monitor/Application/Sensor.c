#include "BH1750.h"
#include "SHT30.h"
#include "ParamSave.h"
#include "Modbus.h"
#include "Modbus_register.h"
void Sensor_Read(void)
{
	    // ================== 读取 SHT30 ==================
    if(SHT30_Read_Data(&temperature, &humidity) == 0)
    {
        //printf("温度: %.2f °C, 湿度: %.2f %%RH\r\n", temperature,humidity);
        __disable_irq();
        Modbus_Reg[REG_TEMP] = (uint16_t)(temperature * 10.0f);
        if(Modbus_Reg[REG_TEMP] > Modbus_Reg[REG_TEMP_ALARM]) // 温度报警判断
            Modbus_Reg[REG_ALARM_STATUS] |= 0x01; // 设置温度报警位
        else
            Modbus_Reg[REG_ALARM_STATUS] &= ~0x01; // 清除温度报警位
        Modbus_Reg[REG_HUMI] = (uint16_t)(humidity * 10.0f);
        if(Modbus_Reg[REG_HUMI] > Modbus_Reg[REG_HUMI_ALARM]) // 湿度报警判断
            Modbus_Reg[REG_ALARM_STATUS] |= 0x02; // 设置湿度报警位
        else
            Modbus_Reg[REG_ALARM_STATUS] &= ~0x02; // 清除湿度报警位
        __enable_irq();
    }
    // // ================== 读取 BH1750 ==================
    light = BH1750_Read_Light();
    if(light >= 0) // 返回值大于等于0说明读取成功
    {
       // printf("光照: %.1f Lux\r\n", light);
        __disable_irq();
        Modbus_Reg[REG_LIGHT] = (uint16_t)(light * 10.0f);
        if(Modbus_Reg[REG_LIGHT] < Modbus_Reg[REG_LIGHT_ALARM]) // 光照报警判断
            Modbus_Reg[REG_ALARM_STATUS] |= 0x04; // 设置光照报警位
        else
            Modbus_Reg[REG_ALARM_STATUS] &= ~0x04; // 清除光照报警位
        __enable_irq();
    }
    if(Save_Flag)
    {
        Save_Params(); // 只要保存一个寄存器，函数内部会备份全部寄存器并写回 Flash
        Save_Flag = 0; // 重置保存标志
    }
    if(Modbus_Reg[REG_ALARM_STATUS]==0x07)
    {
      __disable_irq();
      Modbus_Reg[REG_DEVICE_STATUS] = 0x66; // 设备异常
      __enable_irq();
    }
}
