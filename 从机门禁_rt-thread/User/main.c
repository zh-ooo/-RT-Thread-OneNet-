#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "math.h"
#include "Timer.h"
#include <stdio.h>
#include "Lora.h"
#include "key.h"
#include "gate.h"
#include "AS608.h"
#include "PWM.h"
#include "PID.h"
#include "OPENMV.h"
#include <rtthread.h>

#define SENSOR_SEND_TIME    300    //发送传感器数据时间间隔（单位：100毫秒）
#define FACE_TRACKING_TIME  5      //人脸跟踪时间间隔（单位：100毫秒）

uint8_t AS_status=0;
uint8_t as=0;
uint8_t Pass_status=0;
extern uint8_t Face_status;
extern uint8_t GT;

int x = 0, y = 0;

char str_n[100];

#define THREAD_PRIORITY         10    // 线程优先级
#define THREAD_TIMESLICE        5     // 线程时间片 5ms

// 信号量定义
static struct rt_semaphore sem_sensor_send;
static struct rt_semaphore sem_face_tracking;
static struct rt_semaphore sem_oled_display;

// 线程栈和控制块
static char sns_send_thread_stack[2048];
static struct rt_thread sns_read_thread;

static char face_thread_stack[1024];
static struct rt_thread face_thread;

static char oled_thread_stack[1024];
static struct rt_thread oled_thread;

// 线程入口声明
static void sensor_send_thread(void *param);
static void face_tracking_thread(void *param);
static void oled_display_thread(void *param);

void AS608_SendPacket(void);
uint8_t AS608_read(void);
void Lora_SendData(uint8_t addrH,uint8_t addrL,uint8_t toun,uint8_t begin,uint8_t end,char* Lora_RxPacket);
void Openmv_read(void);

int main(void)
{
    OLED_Init();
    Timer_Init(100,720); //1ms
    Serial_Init(57600);
    Seria2_Init(115200);
    Seria3_Init(9600);
    GATE_Init();
    Key_Init();
    PWM_Init();

    // 初始化信号量
    rt_sem_init(&sem_sensor_send, "sem_sensor", 0, RT_IPC_FLAG_FIFO);	// 数据发送信号量
    rt_sem_init(&sem_face_tracking, "sem_face_trk", 0, RT_IPC_FLAG_FIFO);	// 人脸跟踪信号量
    rt_sem_init(&sem_oled_display, "sem_oled", 0, RT_IPC_FLAG_FIFO);	// OLED显示信号量

	// 数据发送线程（最高优先级）
	rt_thread_init(&sns_read_thread, "sns_send", sensor_send_thread, RT_NULL,
				   &sns_send_thread_stack[0], sizeof(sns_send_thread_stack),
				   THREAD_PRIORITY - 2, THREAD_TIMESLICE);

	// 人脸跟踪线程（中等优先级）
	rt_thread_init(&face_thread, "face_trk", face_tracking_thread, RT_NULL,
				   &face_thread_stack[0], sizeof(face_thread_stack),
				   THREAD_PRIORITY - 1, THREAD_TIMESLICE);

	// OLED显示线程（最低优先级）
	rt_thread_init(&oled_thread, "oled_disp", oled_display_thread, RT_NULL,
				   &oled_thread_stack[0], sizeof(oled_thread_stack),
				   THREAD_PRIORITY, THREAD_TIMESLICE);
	// 启动线程
	rt_thread_startup(&oled_thread);
	rt_thread_startup(&face_thread);
	rt_thread_startup(&sns_read_thread);

	// 主线程可空转
	while (1)
	{
		rt_thread_mdelay(1000);
	}
}

// 数据发送线程（最高优先级）
static void sensor_send_thread(void *param)
{
    while (1)
    {
        rt_sem_take(&sem_sensor_send, RT_WAITING_FOREVER);
        sprintf(str_n,"%u %u",GT,Pass_status);
        Lora_SendData(0x00,0x05,0x14,0xfe,0xff,str_n);
    }
}

// 人脸跟踪线程（中等优先级）
static void face_tracking_thread(void *param)
{
    while (1)
    {
        rt_sem_take(&sem_face_tracking, RT_WAITING_FOREVER);
        Openmv_read();
        AS_status = AS608_read();
        GT = GATE_GetData();
        rt_sem_release(&sem_oled_display); // 通知OLED线程刷新
    }
}

// OLED显示线程（最低优先级）
static void oled_display_thread(void *param)
{
    while (1)
    {
        rt_sem_take(&sem_oled_display, RT_WAITING_FOREVER);
        if(GT==0)
            OLED_ShowString(1,1,"close");
        else if(GT==1)
            OLED_ShowString(1,1,"open ");
        if(AS_status==1)
        {
            Pass_status=1;
            OLED_ShowString(2,1,"pass success");
        }
        else
        {
            Pass_status=0;
            OLED_ShowString(2,1,"pass error  ");
        }
    }
}

void Lora_SendData(uint8_t addrH,uint8_t addrL,uint8_t toun,uint8_t begin,uint8_t end,char* Lora_RxPacket)
{
	Seria3_SendByte(addrH);
	Seria3_SendByte(addrL);
	Seria3_SendByte(toun);
	Seria3_SendByte(begin);
	Seria3_SendString(Lora_RxPacket);
	Seria3_SendByte(end);
}

void AS608_SendPacket(void)
{
    Serial_SendArray(Serial_TxPacket, 12);
}

uint8_t AS608_read(void)
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)==1)
		{
		AS608_SendPacket();
		Delay_ms(800); //延时，等待指纹模组返回数据，这个延时得根据实际程序更改，不然会出现指纹识别后需要再触发一下指纹触碰才会成功
		if(Serial_RxFlag==1)
			{
				if(Serial_RxPacket[8]==0X00)
				{
				  OLED_ShowNum(2,1,1,1);
				  as =1; 
				}
				else//否则就是指纹不匹配
			    {
				  OLED_ShowNum(2,1,0,1);
				  as = 0; 
			    } 
				Serial_RxFlag = 0;//标志位置0
			}
	     }
	return as;
}

void Openmv_read(void)
{
	if (Seria2_RxFlag == 1)
	{
		Seria2_RxFlag = 0;
       	x=Seria2_RxPacket[0];
		y=Seria2_RxPacket[1];
		Face_status=Seria2_RxPacket[3];
	}
	if(x != 0)
	{
		pid_S_X(x,120);
	}
	if(y != 0)
	{
		pid_S_Y(y,80);
	}
	y=0;
	x=0;
}
//接受指纹传来的数据
void USART1_IRQHandler(void)
{
	uint8_t data;
	static uint8_t RxState = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRxPacket = 0;	//定义表示当前接收数据位置的静态变量
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		data = USART_ReceiveData(USART1);				//读取数据寄存器，存放在接收的数据变量
		
		/*当前状态为0，接收数据包包头*/
		if (RxState == 0 && data == 0xEF )
		{
				RxState = 1;			//置下一个状态
				pRxPacket = 0;			//数据包的位置归零
		}
		/*当前状态为1，接收数据包数据*/
		else if (RxState == 1)
		{
			Serial_RxPacket[pRxPacket] = data;	//将数据存入数据包数组的指定位置
			pRxPacket ++;				//数据包的位置自增
			if (pRxPacket >= 15)		//如果收够10个数据
			{
				RxState = 0;			//状态归0
				Serial_RxFlag = 1;		//接收数据包标志位置1，成功接收一个数据包
			}
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		//清除标志位
	}
}
//接受openmv传来的数据
void USART2_IRQHandler(void)
{
	uint8_t data;
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		data = USART_ReceiveData(USART2);
		if (RxState == 0 && data == 0xF3)
		{
			RxState = 1;
			pRxPacket = 0;
		}
		else if (RxState == 1)
		{
			Seria2_RxPacket[pRxPacket] = data;
			pRxPacket ++;
			if(pRxPacket==3)
			{
				RxState=2;
			}
		}
		else if (RxState == 2 && data == 0xF4)
		{
				RxState = 0;
				Seria2_RxFlag = 1;			
		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}

void TIM4_IRQHandler(void)
{
    static uint16_t sensor_send_timer = 0;
    static uint16_t face_tracking_timer = 0;
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {
        if(sensor_send_timer++ == SENSOR_SEND_TIME) {
            sensor_send_timer = 0;
            rt_sem_release(&sem_sensor_send);
        }
        if(face_tracking_timer++ == FACE_TRACKING_TIME) {
            face_tracking_timer = 0;
            rt_sem_release(&sem_face_tracking);
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
