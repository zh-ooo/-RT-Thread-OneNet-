#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	
	GPIO_InitTypeDef GPIO_InitStrure;
    GPIO_InitStrure.GPIO_Mode = GPIO_Mode_AF_PP;//这个是输出模式的选择
    GPIO_InitStrure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;//初始化端口
    GPIO_InitStrure.GPIO_Speed = GPIO_Speed_50MHz;//速度
	GPIO_Init(GPIOA,&GPIO_InitStrure);
	

	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;//CCR
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);

	TIM_Cmd(TIM2, ENABLE);
}	

void PWM_SetCompare1(int16_t Compare)
{
	TIM_SetCompare1(TIM2, Compare);
}
void PWM_SetCompare2(int16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);
}
