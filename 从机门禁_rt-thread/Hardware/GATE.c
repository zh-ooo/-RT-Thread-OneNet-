#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t GT;

void GATE_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t GATE_GetData(void)
{
	if((GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))==1)
	{
		Delay_ms(10);//ȥ����
		if((GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))==1)
		return 1;
	}
	return 0;
}