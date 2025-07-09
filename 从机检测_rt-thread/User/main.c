#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "dht11.h"
#include "AD.h"
#include "math.h"
#include "Timer.h"
#include <stdio.h>
#include "Lora.h"

#include <rtthread.h>

#define SENSOR_READ_TIME	5   //获取传感器数据时间间隔（单位：100毫秒）
#define SENSOR_SEND_TIME	6   //获取传感器数据时间间隔（单位：100毫秒）

static struct rt_semaphore sensor_read_sem;   // 采集信号量
static struct rt_semaphore oled_update_sem;   // OLED刷屏信号量
static struct rt_semaphore sensor_send_sem;   // 数据发送信号量

char str_m[100];

extern float MQ7_PPM, MQ2_PPM, FR;

void Lora_SendData(uint8_t addrH, uint8_t addrL, uint8_t toun, uint8_t begin, uint8_t end, char* Lora_RxPacket);
    
#define THREAD_PRIORITY         10	// 线程优先级
#define THREAD_TIMESLICE        5	// 线程时间片

static char sns_read_thread_stack[2048];	// 传感器读取线程栈
static struct rt_thread sns_read_thread;	// 传感器读取线程
static char sns_oled_thread_stack[1024];	// OLED显示线程栈
static struct rt_thread sns_oled_thread;	// OLED显示线程
static char sns_send_thread_stack[2048];	// 数据发送线程栈
static struct rt_thread sns_send_thread;	// 数据发送线程

static void sensor_read_thread(void *param);
static void oled_display_thread(void *param);
static void sensor_send_thread(void *param);

int main(void)
{
    OLED_Init();
    Timer_Init(1000,7200); //100ms
    Seria3_Init(9600);
    DHT11_Init();
    AD_Init();

    // 初始化信号量
    rt_sem_init(&sensor_read_sem, "sns_sem", 0, RT_IPC_FLAG_FIFO);	// 采集信号量，初始值为0
    rt_sem_init(&oled_update_sem, "oled_sem", 0, RT_IPC_FLAG_FIFO);	// OLED刷新信号量，初始值为0
	rt_sem_init(&sensor_send_sem, "send_sem", 0, RT_IPC_FLAG_FIFO);	// 数据发送信号量，初始值为0
    // 传感器读取线程
    rt_thread_init(&sns_read_thread,
                   "sns_read",
                   sensor_read_thread,
                   RT_NULL,
                   &sns_read_thread_stack[0],
                   sizeof(sns_read_thread_stack),
                   THREAD_PRIORITY - 2, THREAD_TIMESLICE);
    rt_thread_startup(&sns_read_thread);

    // OLED显示线程
    rt_thread_init(&sns_oled_thread,
                   "oled_disp",
                   oled_display_thread,
                   RT_NULL,
                   &sns_oled_thread_stack[0],
                   sizeof(sns_oled_thread_stack),
                   THREAD_PRIORITY , THREAD_TIMESLICE);
    rt_thread_startup(&sns_oled_thread);
				   
    // 数据发送线程
    rt_thread_init(&sns_send_thread,
                   "sns_send",
                   sensor_send_thread,
                   RT_NULL,
                   &sns_send_thread_stack[0],
                   sizeof(sns_send_thread_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&sns_send_thread);
}

/* 传感器读取线程入口 */
static void sensor_read_thread(void *param)
{
    while (1)
    {
        // 等待定时器信号量
        rt_sem_take(&sensor_read_sem, RT_WAITING_FOREVER);

        AD_GetValue();
        DHT11_Read_Data(&wendu, &shidu);
        MQ7_PPM = MQ7_GetData_PPM();
        MQ2_PPM = MQ2_GetData_PPM();
        FR = Fire_GetData();

        // 通知OLED线程刷新
        rt_sem_release(&oled_update_sem);

    }
}

/* OLED显示线程入口 */
static void oled_display_thread(void *param)
{
    while (1)
    {
        // 等待数据更新信号量
        rt_sem_take(&oled_update_sem, RT_WAITING_FOREVER);

        OLED_ShowNum(1, 1, wendu, 3);
        OLED_ShowNum(1, 7, shidu, 3);
        OLED_ShowNum(2, 1, MQ7_PPM, 3);
        OLED_ShowNum(2, 7, MQ2_PPM, 3);
        OLED_ShowNum(3, 1, FR, 3);
    }
}

/* 数据发送线程入口 */
static void sensor_send_thread(void *param)
{
    while (1)
    {
        // 等待定时器信号量
        rt_sem_take(&sensor_send_sem, RT_WAITING_FOREVER);

        sprintf(str_m, "%u %u %.2f %.2f %.2f", wendu, shidu, MQ7_PPM, MQ2_PPM, FR);	
        Lora_SendData(0x00, 0x05, 0x14, 0xfa, 0xff, str_m);
    }
}
//LoRa传输协议+自定义协议
void Lora_SendData(uint8_t addrH, uint8_t addrL, uint8_t toun, uint8_t begin, uint8_t end, char* Lora_RxPacket)
{
    Seria3_SendByte(addrH);
    Seria3_SendByte(addrL);
    Seria3_SendByte(toun);
    Seria3_SendByte(begin);
    Seria3_SendString(Lora_RxPacket);
    Seria3_SendByte(end);
}

void TIM4_IRQHandler(void)
{
    static uint16_t sensor_read_timer = 0;
	static uint16_t sensor_send_timer = 0;
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {	
        if(sensor_read_timer++ == SENSOR_READ_TIME) {
            sensor_read_timer = 0;
            // 释放采集信号量
            rt_sem_release(&sensor_read_sem);
        }
		if(sensor_send_timer++ == SENSOR_SEND_TIME) {
            sensor_send_timer = 0;
            // 释放采集信号量
            rt_sem_release(&sensor_send_sem);
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
