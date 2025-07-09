#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <serial.h>

UART_TypeDef Uart1 = {{0}, 0 ,0};

void Serial_Init(uint32_t Baud)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		//TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		//RX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = Baud;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}


int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

/**
  * 函    数：串口1接收函数
  * 参    数：uint8_t Byte:接收的一个字节
  * 返 回 值：无
  */
void usart1_receive_callback(uint8_t Byte)
{
	Uart1.RxBuffer[Uart1.RxDataCnt++] = Byte;   //接收数据转存
	
	if(Uart1.RxDataCnt > RX1_BUFFER_SIZE) 	//判断接收数据是否溢出
	{
		memset(Uart1.RxBuffer,0x00,sizeof(Uart1.RxBuffer));	//清除缓存
		Uart1.RxDataCnt = 0;	//下标置0，重新开始接收
	}				

	if((Uart1.RxBuffer[Uart1.RxDataCnt - 2] == '\r' && Uart1.RxBuffer[Uart1.RxDataCnt - 1] == '\n')) //判断结束位
	{
		Serial_SendString((char *)Uart1.RxBuffer);			//打印接收到的数据
		memset(Uart1.RxBuffer,0x00,sizeof(Uart1.RxBuffer)); //清空数组
		Uart1.RxDataCnt = 0;								
	}	
}



void uart2_init(uint32_t baud)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					
	
	USART_InitTypeDef USART_InitStructure;				
	USART_InitStructure.USART_BaudRate = baud;			
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	
	USART_InitStructure.USART_Parity = USART_Parity_No;		
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		
	USART_Init(USART2, &USART_InitStructure);				
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);		
	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//指定NVIC线路的抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//指定NVIC线路的响应优先级为0
	NVIC_Init(&NVIC_InitStructure);							
	
	USART_Cmd(USART2, ENABLE);								
}

/**  
  * @简要  无线模块的串口发送一个字节
  * @参数  uint8_t byte：要发送的字节
  * @注意	无
  * @返回值 无  
  */
void uart2_send_byte(uint8_t byte)
{
	USART_SendData(USART2, byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**  
  * @简要  无线模块的串口发送字符串
  * @参数  char *str: 字符数组指针
  * @注意	这里使用了#define 同名替换了wireless_send_data
  * @返回值 无  
  */
void uart2_send_string(char *str)
{
	while(*str != '\0')
	{
		uart2_send_byte(*str++);
	}
}

