#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "BEEP.h"

uint8_t  BEEP_status = 0;

void BEEP_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

void Beep_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	BEEP_status=1;
}

void Beep_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	BEEP_status=0;
}
