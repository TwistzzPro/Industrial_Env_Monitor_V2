/*掉电保存文件*/
#include "ParamSave.h"

#define PARAM_SAVE_ADDRESS 0x0800FC00 // 这是STM32F103C8T6的最后一个页的起始地址，掉电保存数据将存储在这里
#define Reg_Num 10   //寄存器数量
#define Regs_Size 20 //每组数据所占用的字节
#define Write_Num 51 //每页可写入的次数

// 一共10个寄存器，每一个是两个字节，每写入一次要占用20个字节，一页有1024个字节，可以写51次，写满51次之后在执行擦除
// 那每次上电之后读取数据也要偏移地址
FLASH_EraseInitTypeDef EraseInitStruct;

uint32_t PageError = 0;
static uint8_t Current_Number;

void Scan_Flash(void)               //扫描flash
{
    Current_Number = 0;
    for(uint8_t i = 0 ; i < Write_Num ; i++)
    {
        uint32_t Flash_Addr = PARAM_SAVE_ADDRESS + i * 20;
        uint16_t Flash_Data = *(__IO uint16_t *)Flash_Addr;
        if(Flash_Data == 0xFFFF)
        {
            Current_Number = i;
            return;
        }
    }
    Current_Number = Write_Num;
}

void Load_Params(void)
{
    Scan_Flash();
    if(Current_Number == 0)
    {
        for(uint8_t i = 0; i < Reg_Num; i++)
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
    }
        else
    {
        // 从Flash读取参数值
        uint8_t Read_Slot = Current_Number - 1;
        for(uint8_t i = 0 ; i < Reg_Num ; i++)
        {
            uint16_t *param_value = (uint16_t *)(PARAM_SAVE_ADDRESS + Read_Slot * Reg_Num + i * 2);
            Modbus_Reg[i] = *param_value; // 从Flash读取参数值到Modbus寄存器数组中
        }

    }
}

// void Write_Params(void)              //滚动写入                 有个问题就是掉电之后，num也会清零，怎么办
// {
//     HAL_FLASH_Unlock();
//     for(uint8_t i = 0; i < Reg_Num ; i++)
//     {
//         HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
//                           PARAM_SAVE_ADDRESS + Current_Number * 20  + i * 2,
//                           Modbus_Reg[i]);
//     }
//     HAL_FLASH_Lock();
//     Current_Number++;
// }

void Save_Params(void)
{
    HAL_FLASH_Unlock();
    if(Current_Number >= Write_Num)
    {
        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.PageAddress = PARAM_SAVE_ADDRESS;
        EraseInitStruct.NbPages     = 1;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
        {
            Current_Number = 0;
        }
        else
        {
            HAL_FLASH_Lock();
            return;
        }
    }
    for(uint8_t i = 0; i < Reg_Num ; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                          PARAM_SAVE_ADDRESS + Current_Number * 20  + i * 2,
                          Modbus_Reg[i]);
    }
        Current_Number ++;
        HAL_FLASH_Lock();
}

