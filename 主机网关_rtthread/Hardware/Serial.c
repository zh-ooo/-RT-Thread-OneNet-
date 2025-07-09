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
  * ��    ��������1���պ���
  * ��    ����uint8_t Byte:���յ�һ���ֽ�
  * �� �� ֵ����
  */
void usart1_receive_callback(uint8_t Byte)
{
	Uart1.RxBuffer[Uart1.RxDataCnt++] = Byte;   //��������ת��
	
	if(Uart1.RxDataCnt > RX1_BUFFER_SIZE) 	//�жϽ��������Ƿ����
	{
		memset(Uart1.RxBuffer,0x00,sizeof(Uart1.RxBuffer));	//�������
		Uart1.RxDataCnt = 0;	//�±���0�����¿�ʼ����
	}				

	if((Uart1.RxBuffer[Uart1.RxDataCnt - 2] == '\r' && Uart1.RxBuffer[Uart1.RxDataCnt - 1] == '\n')) //�жϽ���λ
	{
		Serial_SendString((char *)Uart1.RxBuffer);			//��ӡ���յ�������
		memset(Uart1.RxBuffer,0x00,sizeof(Uart1.RxBuffer)); //�������
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//ָ��NVIC��·����ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//ָ��NVIC��·����Ӧ���ȼ�Ϊ0
	NVIC_Init(&NVIC_InitStructure);							
	
	USART_Cmd(USART2, ENABLE);								
}

/**  
  * @��Ҫ  ����ģ��Ĵ��ڷ���һ���ֽ�
  * @����  uint8_t byte��Ҫ���͵��ֽ�
  * @ע��	��
  * @����ֵ ��  
  */
void uart2_send_byte(uint8_t byte)
{
	USART_SendData(USART2, byte);		//���ֽ�����д�����ݼĴ�����д���USART�Զ�����ʱ����
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);	//�ȴ��������
	/*�´�д�����ݼĴ������Զ����������ɱ�־λ���ʴ�ѭ�������������־λ*/
}

/**  
  * @��Ҫ  ����ģ��Ĵ��ڷ����ַ���
  * @����  char *str: �ַ�����ָ��
  * @ע��	����ʹ����#define ͬ���滻��wireless_send_data
  * @����ֵ ��  
  */
void uart2_send_string(char *str)
{
	while(*str != '\0')
	{
		uart2_send_byte(*str++);
	}
}

