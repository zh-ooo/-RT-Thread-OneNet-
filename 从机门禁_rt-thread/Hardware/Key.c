#include "stm32f10x.h"                  // Device header
#include "Delay.h"


void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_12| GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	uint8_t keyNum;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0);
		Delay_ms(20);
		keyNum =  2;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);
		Delay_ms(20);
		keyNum =  1;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);
		Delay_ms(20);
		keyNum =  4;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0);
		Delay_ms(20);
		keyNum =  3;
	}
	return keyNum;
}
