#include "MYIIC.h"

/* * 微秒级延时函数 (关键！)
 * 软件 IIC 需要极短的延时来控制通信频率 (通常是 100kHz 或 400kHz)。
 * HAL_Delay() 是毫秒级的，太慢了。这里提供一个简单的粗略微秒延时。
 * 注意：如果你的系统主频不是 72MHz，或者用了 FreeRTOS，这里可能需要调整。
 */
void IIC_Delay(void)
{
    volatile uint8_t i = 200; // volatile防止编译器优化掉循环；值可根据示波器微调
    while(i--);
}

// 初始化IIC
void IIC_Init(void)
{
    // 因为使用 CubeMX 生成的代码，MX_GPIO_Init() 已经初始化了 PB8 和 PB9
    // 所以这里只需要确保总线释放（SCL和SDA都拉高）即可
    IIC_SCL(1);
    IIC_SDA(1);
}

// 产生IIC起始信号
// 时序：SCL为高电平时，SDA由高变低
void IIC_Start(void)
{
    IIC_SDA(1);	  
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(0); // START:当SCL为高时，SDA拉低
    IIC_Delay();
    IIC_SCL(0); // 钳住I2C总线，准备发送或接收数据
}	  

// 产生IIC停止信号
// 时序：SCL为高电平时，SDA由低变高
void IIC_Stop(void)
{
    IIC_SCL(0);
    IIC_SDA(0); // STOP:当SCL为高时，SDA拉高
    IIC_Delay();
    IIC_SCL(1); 
    IIC_Delay();
    IIC_SDA(1); // 发送I2C总线结束信号
}

// 等待应答信号到来
// 返回值：1，接收应答失败；0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;

    IIC_SDA(1); // 主机释放SDA线（开漏模式下输出1就是高阻态，交由从机控制）
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();	 // 等待SCL真正升到高电平

    while(READ_SDA) // 等待从机把SDA拉低
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();
            return 1; // 超时，没等到ACK
        }
        IIC_Delay(); // 【修复】循环体内加延时，给SHT30足够时间响应
    }
    IIC_SCL(0); // 时钟输出0，结束ACK检查
    return 0;
} 

// 产生ACK应答
void IIC_Ack(void)
{
    IIC_SCL(0);
    IIC_SDA(0); // 主机把SDA拉低
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
}

// 不产生ACK应答		    
void IIC_NAck(void)
{
    IIC_SCL(0);
    IIC_SDA(1); // 主机把SDA拉高
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
}					 				     

// IIC发送一个字节
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
    IIC_SCL(0); // 拉低时钟开始数据传输
    for(t=0; t<8; t++)
    {              
        // 将数据的最高位准备好在SDA线上
        if((txd & 0x80) >> 7)
            IIC_SDA(1);
        else
            IIC_SDA(0);
        txd <<= 1; // 数据左移一位
        IIC_Delay();
        
        IIC_SCL(1); // SCL拉高，从机会在此时读取SDA上的电平
        IIC_Delay(); 
        IIC_SCL(0); // SCL拉低，允许SDA电平改变
    }	 
} 	    

// 读1个字节
// ack=1时，发送ACK；ack=0，发送nACK
uint8_t IIC_Read_Byte(uint8_t ack)
{
    uint8_t i, receive = 0;
    IIC_SDA(1); // 主机释放SDA线，准备读取

    for(i=0; i<8; i++ )
    {
        IIC_SCL(0);
        IIC_Delay();
        IIC_SCL(1); // SCL拉高
        IIC_Delay(); // 【修复】等待SCL真正升到高电平 + SHT30输出数据稳定
        receive <<= 1;
        if(READ_SDA) receive++; // 在SCL高电平中间点读取SDA
        IIC_Delay();
    }
    if (!ack)
        IIC_NAck(); // 读完最后一个字节发送NACK
    else
        IIC_Ack();  // 读完不是最后一个字节发送ACK

    return receive;
}


