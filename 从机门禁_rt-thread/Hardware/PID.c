#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "OLED.h"

int16_t Err_X = 0,Err_Y = 0;

int16_t x_pwm = 0,now_x;
int16_t y_pwm = 0,now_y;
int16_t ks = 1500;
float Err_S_y = 0,last_err_S_Y = 0,integral_Y = 0,p_S_Y = -1.7,i_S_Y = -0.15,d_S_Y = -0.36;
float Err_S_X = 0,last_err_S_X = 0,integral_X = 0,p_S_X = 1.73,i_S_X = 0.15,d_S_X = 0;
void pid_S_Y(float true_S, float tar_S)
{
    Err_S_y = tar_S - true_S;
	integral_Y += Err_S_y;
	y_pwm=p_S_Y * Err_S_y+d_S_Y*(Err_S_y-last_err_S_Y)+i_S_Y * integral_Y;
    last_err_S_Y = Err_S_y;
	if(y_pwm > 1000)
	{
		y_pwm = 1000;
	}
	else if(y_pwm < -1000)
	{
		y_pwm = -1000;
	}
	now_y = ks + y_pwm;
    PWM_SetCompare2(now_y);
}

void pid_S_X(float true_S, float tar_S)
{
	Err_S_X = tar_S - true_S;
	integral_X += Err_S_X;
	x_pwm=p_S_X * Err_S_X+d_S_X*(Err_S_X-last_err_S_X)+i_S_X * integral_X;
    last_err_S_X = Err_S_X;
	if(x_pwm > 1000)
	{
		x_pwm = 1000;
	}
	else if(x_pwm < -1000)
	{
		x_pwm = -1000;
	}
	now_x = ks + x_pwm;
    PWM_SetCompare1(now_x);
}