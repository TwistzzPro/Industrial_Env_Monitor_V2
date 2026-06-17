#ifndef __MYIIC_H
#define __MYIIC_H

#include "main.h" // 必须包含 main.h，因为 CubeMX 生成的引脚宏定义在里面

/* * 引脚操作宏定义 
 * 这里默认 SCL 是 PB6，SDA 是 PB7。如果你在 CubeMX 里改了名字，这里对应修改即可。
 */
#define IIC_SCL(n)  (n ? HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET))
#define IIC_SDA(n)  (n ? HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_RESET))
#define READ_SDA    HAL_GPIO_ReadPin(IIC_SDA_GPIO_Port, IIC_SDA_Pin)

#define IIC_SCL1(n)  (n ? HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET))
#define IIC_SDA1(n)  (n ? HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_RESET))
#define READ_SDA1    HAL_GPIO_ReadPin(IIC_SDA_GPIO_Port, IIC_SDA_Pin)
/* IIC 所有操作函数声明 */
void IIC_Init(void);                // 初始化IIC的IO口
void IIC_Start(void);               // 发送IIC开始信号
void IIC_Stop(void);                // 发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);    // IIC发送一个字节
uint8_t IIC_Read_Byte(uint8_t ack); // IIC读取一个字节
uint8_t IIC_Wait_Ack(void);         // IIC等待ACK信号
void IIC_Ack(void);                 // IIC发送ACK信号
void IIC_NAck(void);                // IIC不发送ACK信号
void IIC_Delay(void);               // IIC延时函数

#endif
