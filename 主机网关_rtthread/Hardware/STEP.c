#include "stm32f10x.h"                  // Device header
#include "delay.h"

void Stepper_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;	 //DIR STEP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
}

void Stepper1_Run(uint8_t dir,uint32_t num,uint32_t speed)
{
	uint32_t i;
	if(dir==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_4);
	}
	if(dir==0)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_4);
	}
	for(i=0;i<(num*3200);i++)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_3);
		Delay_us(speed);
		GPIO_ResetBits(GPIOB,GPIO_Pin_3);
		Delay_us(speed);
	}
}

void Stepper2_Run(uint8_t dir,uint32_t num,uint32_t speed)
{
	uint32_t i;
	if(dir==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_6);
	}
	if(dir==0)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	}
	for(i=0;i<(num*3200);i++)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
		Delay_us(speed);
		GPIO_ResetBits(GPIOB,GPIO_Pin_5);
		Delay_us(speed);
	}
}
