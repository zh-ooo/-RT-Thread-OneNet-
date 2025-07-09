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

#define SENSOR_READ_TIME	5   //��ȡ����������ʱ��������λ��100���룩
#define SENSOR_SEND_TIME	6   //��ȡ����������ʱ��������λ��100���룩

static struct rt_semaphore sensor_read_sem;   // �ɼ��ź���
static struct rt_semaphore oled_update_sem;   // OLEDˢ���ź���
static struct rt_semaphore sensor_send_sem;   // ���ݷ����ź���

char str_m[100];

extern float MQ7_PPM, MQ2_PPM, FR;

void Lora_SendData(uint8_t addrH, uint8_t addrL, uint8_t toun, uint8_t begin, uint8_t end, char* Lora_RxPacket);
    
#define THREAD_PRIORITY         10	// �߳����ȼ�
#define THREAD_TIMESLICE        5	// �߳�ʱ��Ƭ

static char sns_read_thread_stack[2048];	// ��������ȡ�߳�ջ
static struct rt_thread sns_read_thread;	// ��������ȡ�߳�
static char sns_oled_thread_stack[1024];	// OLED��ʾ�߳�ջ
static struct rt_thread sns_oled_thread;	// OLED��ʾ�߳�
static char sns_send_thread_stack[2048];	// ���ݷ����߳�ջ
static struct rt_thread sns_send_thread;	// ���ݷ����߳�

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

    // ��ʼ���ź���
    rt_sem_init(&sensor_read_sem, "sns_sem", 0, RT_IPC_FLAG_FIFO);	// �ɼ��ź�������ʼֵΪ0
    rt_sem_init(&oled_update_sem, "oled_sem", 0, RT_IPC_FLAG_FIFO);	// OLEDˢ���ź�������ʼֵΪ0
	rt_sem_init(&sensor_send_sem, "send_sem", 0, RT_IPC_FLAG_FIFO);	// ���ݷ����ź�������ʼֵΪ0
    // ��������ȡ�߳�
    rt_thread_init(&sns_read_thread,
                   "sns_read",
                   sensor_read_thread,
                   RT_NULL,
                   &sns_read_thread_stack[0],
                   sizeof(sns_read_thread_stack),
                   THREAD_PRIORITY - 2, THREAD_TIMESLICE);
    rt_thread_startup(&sns_read_thread);

    // OLED��ʾ�߳�
    rt_thread_init(&sns_oled_thread,
                   "oled_disp",
                   oled_display_thread,
                   RT_NULL,
                   &sns_oled_thread_stack[0],
                   sizeof(sns_oled_thread_stack),
                   THREAD_PRIORITY , THREAD_TIMESLICE);
    rt_thread_startup(&sns_oled_thread);
				   
    // ���ݷ����߳�
    rt_thread_init(&sns_send_thread,
                   "sns_send",
                   sensor_send_thread,
                   RT_NULL,
                   &sns_send_thread_stack[0],
                   sizeof(sns_send_thread_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&sns_send_thread);
}

/* ��������ȡ�߳���� */
static void sensor_read_thread(void *param)
{
    while (1)
    {
        // �ȴ���ʱ���ź���
        rt_sem_take(&sensor_read_sem, RT_WAITING_FOREVER);

        AD_GetValue();
        DHT11_Read_Data(&wendu, &shidu);
        MQ7_PPM = MQ7_GetData_PPM();
        MQ2_PPM = MQ2_GetData_PPM();
        FR = Fire_GetData();

        // ֪ͨOLED�߳�ˢ��
        rt_sem_release(&oled_update_sem);

    }
}

/* OLED��ʾ�߳���� */
static void oled_display_thread(void *param)
{
    while (1)
    {
        // �ȴ����ݸ����ź���
        rt_sem_take(&oled_update_sem, RT_WAITING_FOREVER);

        OLED_ShowNum(1, 1, wendu, 3);
        OLED_ShowNum(1, 7, shidu, 3);
        OLED_ShowNum(2, 1, MQ7_PPM, 3);
        OLED_ShowNum(2, 7, MQ2_PPM, 3);
        OLED_ShowNum(3, 1, FR, 3);
    }
}

/* ���ݷ����߳���� */
static void sensor_send_thread(void *param)
{
    while (1)
    {
        // �ȴ���ʱ���ź���
        rt_sem_take(&sensor_send_sem, RT_WAITING_FOREVER);

        sprintf(str_m, "%u %u %.2f %.2f %.2f", wendu, shidu, MQ7_PPM, MQ2_PPM, FR);	
        Lora_SendData(0x00, 0x05, 0x14, 0xfa, 0xff, str_m);
    }
}
//LoRa����Э��+�Զ���Э��
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
            // �ͷŲɼ��ź���
            rt_sem_release(&sensor_read_sem);
        }
		if(sensor_send_timer++ == SENSOR_SEND_TIME) {
            sensor_send_timer = 0;
            // �ͷŲɼ��ź���
            rt_sem_release(&sensor_send_sem);
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
