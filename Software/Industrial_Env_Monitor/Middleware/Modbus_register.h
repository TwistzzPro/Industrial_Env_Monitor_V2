#ifndef __MODBUS_REGISTER_H
#define __MODBUS_REGISTER_H

#define REG_TEMP          0
#define REG_HUMI          1
#define REG_LIGHT         2

#define REG_DEVICE_STATUS 3
#define REG_FW_VERSION    4
#define REG_SLAVE_ADDR    5

#define REG_TEMP_ALARM      6      //温度阈值
#define REG_HUMI_ALARM      7      //湿度阈值
#define REG_LIGHT_ALARM     8      //光照阈值
#define REG_ALARM_STATUS    9      //bit0: 温度报警  bit1: 湿度报警  bit2: 光照报警

#endif
