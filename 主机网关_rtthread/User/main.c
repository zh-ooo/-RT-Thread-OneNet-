#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "BEEP.h"
#include "LED.h"
#include "Driver_wireless.h"
#include "Timer.h"
#include "STEP.h"
#include "LORA.h"

#include <rtthread.h>

#define WIRELESS_SEND_TIME 	1000	//上报数据时间间隔（单位：毫秒）
#define SENSOR_READ_TIME	1000    //获取传感器数据时间间隔（单位：毫秒）
#define USER_MOVE_TIME		1000    	//执行报警数据时间间隔（单位：毫秒）

char str_n[100],str_m[100];

static struct rt_semaphore sensor_read_sem;		//读取传感器相关信号量
static struct rt_semaphore wireless_send_sem;	//无线发送相关信号量
static struct rt_semaphore user_move_sem;	//无线发送相关信号量

#define SENSOR_THREAD_PRIORITY           12	 // 线程优先级 10
#define SENSOR_THREAD_TIMESLICE          5    // 线程时间片 5ms

#define WIRELESS_THREAD_PRIORITY         11	 // 线程优先级 11
#define WIRELESS_THREAD_TIMESLICE        5    // 线程时间片 5ms

#define USER_THREAD_PRIORITY         10	 // 线程优先级 12
#define USER_THREAD_TIMESLICE        5    // 线程时间片 5ms

static char sns_read_thread_stack[2048];	 // 传感器读取线程栈空间
static struct rt_thread sns_read_thread;	 // 传感器读取线程控制块

static char wireless_send_thread_stack[2048];	 // 无线发送线程栈空间
static struct rt_thread wireless_thread;	 // 无线发送程控制块

static char user_thread_stack[1024];	 // 无线发送线程栈空间
static struct rt_thread user_thread;	 // 无线发送程控制块

static void sensor_read_thread(void *param);	 // 传感器读取线程入口
static void wireless_send_thread(void *param);	 // 无线发送线程入口
static void user_move_thread(void *param);	 // 无线发送线程入口

void Report_Move(uint8_t wendu_r,uint8_t shidu_r,uint8_t MQ7_PPM_r,uint8_t MQ2_PPM_r,uint16_t FR_r);

int main(void)
{
	OLED_Init();
	LED_Init();
	BEEP_Init();
	Serial_Init(115200);
	Seria3_Init(9600);
	Timer_Init(100,720); //1ms
	wireless_init();
	OLED_Clear();
	Stepper_Init();

    // 初始化信号量
    rt_sem_init(&sensor_read_sem, "sensor_sem", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&wireless_send_sem, "wireless_sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&user_move_sem, "user_sem", 0, RT_IPC_FLAG_FIFO);

	// 数据读取线程（最低优先级）
    rt_thread_init(&sns_read_thread,
		"sns_read",
		sensor_read_thread,
		RT_NULL,
		&sns_read_thread_stack[0],
		sizeof(sns_read_thread_stack),
		SENSOR_THREAD_PRIORITY , SENSOR_THREAD_TIMESLICE);
	// 无线发送线程（中间优先级）
    rt_thread_init(&wireless_thread,
		"wireless",
		wireless_send_thread,
		RT_NULL,
		&wireless_send_thread_stack[0],
		sizeof(wireless_send_thread_stack),
		WIRELESS_THREAD_PRIORITY , WIRELESS_THREAD_TIMESLICE);
	// 报警行动线程（最高优先级）
    rt_thread_init(&user_thread,
		"user",
		user_move_thread,
		RT_NULL,
		&user_thread_stack[0],
		sizeof(user_thread_stack),
		USER_THREAD_PRIORITY , USER_THREAD_TIMESLICE);
	// 线程初始化后会自动启动
    rt_thread_startup(&sns_read_thread);
    rt_thread_startup(&wireless_thread);
	rt_thread_startup(&user_thread);

}

/* 传感器读取线程入口 */
static void sensor_read_thread(void *param)
{
	while (1) // 线程循环
	{
		// 等待信号量
		rt_sem_take(&sensor_read_sem, RT_WAITING_FOREVER);
		sscanf(str_n, "%u %u", &GT, &Pass_status);
		sscanf(str_m, "%u %u %f %f %f", &wendu, &shidu, &MQ7_PPM, &MQ2_PPM, &FR);
		OLED_ShowNum(1, 1, wendu, 3);
		OLED_ShowNum(1, 8, shidu, 3);
		OLED_ShowNum(2, 1, MQ2_PPM, 4);
		OLED_ShowNum(2, 8, MQ7_PPM, 4);
		OLED_ShowNum(3, 1, FR, 4);
		OLED_ShowNum(4, 1, GT, 1);
		OLED_ShowNum(4, 8, Pass_status, 1);
	}
}
/* 无线发送线程入口 */
static void wireless_send_thread(void *param)
{
	while (1)
	{
		// 等待信号量，替代原来的flag判断
		rt_sem_take(&wireless_send_sem, RT_WAITING_FOREVER);
		// 无线发送业务
		wireless_system_handler();		// 执行无线模块相关事件
		wireless_onenet_data_handler(); // 处理有关onenet的数据
		if ((WirelessStatus.error_code & ERROR_MQTT_CONNECT) == 0)
			wireless_publish_data();			 // 发布数据测试
		if (wireless_get_receive_flag() == W_OK) // 无线模块接收到数据
		{
			wireless_receive_data_handler(); // 接收数据处理函数
		}
	}
}
/* 报警行动线程入口 */
static void user_move_thread(void *param)
{
	while (1)
	{
		rt_sem_take(&user_move_sem, RT_WAITING_FOREVER);
		Report_Move(50, 50, 3, 3, 4000);
	}
}

/**
  * @简要：USART1中断函数
  * @参数：无
  * @注意：此函数为中断函数，无需调用，中断触发后自动执行
  *        函数名为预留的指定名称，可以从启动文件复制
  *        请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  *@返回值 无 
  */
void USART1_IRQHandler(void)
{
	uint8_t data = 0;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		
	{
		data = USART_ReceiveData(USART1);
		usart1_receive_callback(data);			
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);																	
	}
}

/**  
  * @简要  串口2接收中断服务函数
  * @参数  	无
  * @注意	将无线模块接收函数放在此中断的接收中
  * @返回值 无  
  */
void USART2_IRQHandler(void)
{
	uint8_t data = 0;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)		
	{
		data = USART_ReceiveData(USART2);	
		wireless_receive_callback(data);	/* 将无线模块接收数据函数放在串口接收中断中，接收数据 */		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);																	
	}
}
/**  
  * @简要  串口3接收从机传来的数据
  * @参数  	无
  * @返回值 无  
  */
void USART3_IRQHandler(void) //中断里执行接受数据的功能
{
	static uint8_t pRxPacket1 = 0;
	static uint8_t action1 = 0;
	static uint8_t pRxPacket2 = 0;
	static uint8_t action2 = 0;
	uint8_t data=0;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		data = USART_ReceiveData(USART3);
		
		if (action1==0 && data==0xfa)
		{
			action1=1;
			pRxPacket1 = 0;
		}
		else if (action1==1 && data!=0xff)
		{
			str_m[pRxPacket1] = data;//把接收到的数据放到数组里面
			pRxPacket1 ++;
		}
		else if(action1==1 && data==0xff)
		{
			action1=0;
			str_m[pRxPacket1] = '\0';
			pRxPacket1 = 0;	
		}
		
		if (action2==0 && data==0xfe)
		{
			action2=1;
			pRxPacket2 = 0;
		}
		else if (action2==1 && data!=0xff)
		{
			str_n[pRxPacket2] = data;//把接收到的数据放到数组里面
			pRxPacket2 ++;
		}
		else if(action2==1 && data==0xff)
		{
			action2=0;
			str_n[pRxPacket2] = '\0';
			pRxPacket2 = 0;	
		}
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}

void TIM4_IRQHandler(void)
{
	static uint16_t wireless_send_timer = 0;
	static uint16_t sensor_read_timer = 0;
	static uint16_t user_move_timer = 0;
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		if (wireless_send_timer++ == WIRELESS_SEND_TIME)
		{
			wireless_send_timer = 0;
			rt_sem_release(&wireless_send_sem);	//发送信号量
		}
		if (sensor_read_timer++ == SENSOR_READ_TIME)
		{
			sensor_read_timer = 0;
			rt_sem_release(&sensor_read_sem);  //发送信号量
		}
		if (user_move_timer++ == USER_MOVE_TIME)
		{
			user_move_timer = 0;
			rt_sem_release(&user_move_sem);  //发送信号量
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}
//报警行动
void Report_Move(uint8_t wendu_r,uint8_t shidu_r,uint8_t MQ7_PPM_r,uint8_t MQ2_PPM_r,uint16_t FR_r)
{
	static uint8_t D0=0;
	static uint8_t D1=0;
	static uint8_t M0=0;
	static uint8_t M1=0;	
	if(wendu>wendu_r || shidu>shidu_r||(Pass_status==0&&GT==1))
	{
		D0++;
		if(D0==5)
		{
			D0=0;
			Beep_ON();
			LED_ON();		
		}

	}
	else
	{
		D1++;
		if(D1==5)
		{
			D1=0;
			Beep_OFF();
			LED_OFF();	
		}
	}		
	if((MQ7_PPM>MQ7_PPM_r)||(FR>FR_r)||(MQ2_PPM>MQ2_PPM_r))
	{
		M0++;
		if(M0==5)
		{
			M0=0;
			Stepper2_Run(1,4,100);
		}
	}
	if(Pass_status==1)
	{
		M1++;
		if(M1==2)
		{
			M1=0;
			Stepper1_Run(1,4,100);
		}
	}
}
