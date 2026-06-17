/*掉电保存文件*/
#include "ParamSave.h"

#define PARAM_SAVE_ADDRESS 0x0800FC00 // 这是STM32F103C8T6的最后一个扇区地址，掉电保存数据将存储在这里
FLASH_EraseInitTypeDef EraseInitStruct;

uint32_t PageError = 0;

void Load_Params(void)
{
    for(uint8_t i = 0; i < 10; i++)
    {
        uint16_t *param_value = (uint16_t *)(PARAM_SAVE_ADDRESS + i*2); // 每个参数占用2字节
        if(*param_value == 0xFFFF) // 判断是否有有效数据，0xFFFF表示未写入过
        {
            // Flash 从未写过 → 用默认值
            if (i == REG_DEVICE_STATUS)
                Modbus_Reg[i] = 0x04;       // 默认设备状态
            else if (i == REG_FW_VERSION)
                Modbus_Reg[i] = 0x05;
            else if (i == REG_SLAVE_ADDR)
                Modbus_Reg[i] = 0x01;       // 默认从站地址
            else if (i == REG_TEMP_ALARM)
                Modbus_Reg[i] = 300;        // 默认温度报警阈值 30.0°C
            else if (i == REG_HUMI_ALARM)
                Modbus_Reg[i] = 700;        // 默认湿度报警阈值 70.0%RH
            else if (i == REG_LIGHT_ALARM)
                Modbus_Reg[i] = 1000;       // 默认光照报警阈值 100.0 Lux
            else
                Modbus_Reg[i] = 0;
        }
        else
        {
            // 从Flash读取参数值
            Modbus_Reg[i] = *param_value; // 从Flash读取参数值到Modbus寄存器数组中
        }
    }
}
void Save_Params(uint16_t Reg_Address)
{
    // 1. 擦除前先备份全部寄存器（擦除会让 Flash 全部变 0xFFFF）
    uint16_t backup[10];
    for (uint8_t i = 0; i < 10; i++)
    {
        // 直接读 Flash 当前值作为备份
        uint16_t *flash_addr = (uint16_t *)(PARAM_SAVE_ADDRESS + i * 2);
        backup[i] = *flash_addr;
    }
    // 把要修改的寄存器更新到备份中
    backup[Reg_Address] = Modbus_Reg[Reg_Address];

    // 2. 解锁并擦除
    HAL_FLASH_Unlock();
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = PARAM_SAVE_ADDRESS;
    EraseInitStruct.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return;
    }

    // 3. 全部写回（只写非 0xFFFF 的有效值）
    for (uint8_t i = 0; i < 10; i++)
    {
        if (backup[i] != 0xFFFF)
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                                  PARAM_SAVE_ADDRESS + i * 2,
                                  backup[i]) != HAL_OK)
            {
                HAL_FLASH_Lock();
                return;
            }
        }
    }

    HAL_FLASH_Lock();
}

