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

#define SENSOR_SEND_TIME    300    //���ʹ���������ʱ��������λ��100���룩
#define FACE_TRACKING_TIME  5      //��������ʱ��������λ��100���룩

uint8_t AS_status=0;
uint8_t as=0;
uint8_t Pass_status=0;
extern uint8_t Face_status;
extern uint8_t GT;

int x = 0, y = 0;

char str_n[100];

#define THREAD_PRIORITY         10    // �߳����ȼ�
#define THREAD_TIMESLICE        5     // �߳�ʱ��Ƭ 5ms

// �ź�������
static struct rt_semaphore sem_sensor_send;
static struct rt_semaphore sem_face_tracking;
static struct rt_semaphore sem_oled_display;

// �߳�ջ�Ϳ��ƿ�
static char sns_send_thread_stack[2048];
static struct rt_thread sns_read_thread;

static char face_thread_stack[1024];
static struct rt_thread face_thread;

static char oled_thread_stack[1024];
static struct rt_thread oled_thread;

// �߳��������
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

    // ��ʼ���ź���
    rt_sem_init(&sem_sensor_send, "sem_sensor", 0, RT_IPC_FLAG_FIFO);	// ���ݷ����ź���
    rt_sem_init(&sem_face_tracking, "sem_face_trk", 0, RT_IPC_FLAG_FIFO);	// ���������ź���
    rt_sem_init(&sem_oled_display, "sem_oled", 0, RT_IPC_FLAG_FIFO);	// OLED��ʾ�ź���

	// ���ݷ����̣߳�������ȼ���
	rt_thread_init(&sns_read_thread, "sns_send", sensor_send_thread, RT_NULL,
				   &sns_send_thread_stack[0], sizeof(sns_send_thread_stack),
				   THREAD_PRIORITY - 2, THREAD_TIMESLICE);

	// ���������̣߳��е����ȼ���
	rt_thread_init(&face_thread, "face_trk", face_tracking_thread, RT_NULL,
				   &face_thread_stack[0], sizeof(face_thread_stack),
				   THREAD_PRIORITY - 1, THREAD_TIMESLICE);

	// OLED��ʾ�̣߳�������ȼ���
	rt_thread_init(&oled_thread, "oled_disp", oled_display_thread, RT_NULL,
				   &oled_thread_stack[0], sizeof(oled_thread_stack),
				   THREAD_PRIORITY, THREAD_TIMESLICE);
	// �����߳�
	rt_thread_startup(&oled_thread);
	rt_thread_startup(&face_thread);
	rt_thread_startup(&sns_read_thread);

	// ���߳̿ɿ�ת
	while (1)
	{
		rt_thread_mdelay(1000);
	}
}

// ���ݷ����̣߳�������ȼ���
static void sensor_send_thread(void *param)
{
    while (1)
    {
        rt_sem_take(&sem_sensor_send, RT_WAITING_FOREVER);
        sprintf(str_n,"%u %u",GT,Pass_status);
        Lora_SendData(0x00,0x05,0x14,0xfe,0xff,str_n);
    }
}

// ���������̣߳��е����ȼ���
static void face_tracking_thread(void *param)
{
    while (1)
    {
        rt_sem_take(&sem_face_tracking, RT_WAITING_FOREVER);
        Openmv_read();
        AS_status = AS608_read();
        GT = GATE_GetData();
        rt_sem_release(&sem_oled_display); // ֪ͨOLED�߳�ˢ��
    }
}

// OLED��ʾ�̣߳�������ȼ���
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
		Delay_ms(800); //��ʱ���ȴ�ָ��ģ�鷵�����ݣ������ʱ�ø���ʵ�ʳ�����ģ���Ȼ�����ָ��ʶ�����Ҫ�ٴ���һ��ָ�ƴ����Ż�ɹ�
		if(Serial_RxFlag==1)
			{
				if(Serial_RxPacket[8]==0X00)
				{
				  OLED_ShowNum(2,1,1,1);
				  as =1; 
				}
				else//�������ָ�Ʋ�ƥ��
			    {
				  OLED_ShowNum(2,1,0,1);
				  as = 0; 
			    } 
				Serial_RxFlag = 0;//��־λ��0
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
//����ָ�ƴ���������
void USART1_IRQHandler(void)
{
	uint8_t data;
	static uint8_t RxState = 0;		//�����ʾ��ǰ״̬��״̬�ľ�̬����
	static uint8_t pRxPacket = 0;	//�����ʾ��ǰ��������λ�õľ�̬����
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		//�ж��Ƿ���USART1�Ľ����¼��������ж�
	{
		data = USART_ReceiveData(USART1);				//��ȡ���ݼĴ���������ڽ��յ����ݱ���
		
		/*��ǰ״̬Ϊ0���������ݰ���ͷ*/
		if (RxState == 0 && data == 0xEF )
		{
				RxState = 1;			//����һ��״̬
				pRxPacket = 0;			//���ݰ���λ�ù���
		}
		/*��ǰ״̬Ϊ1���������ݰ�����*/
		else if (RxState == 1)
		{
			Serial_RxPacket[pRxPacket] = data;	//�����ݴ������ݰ������ָ��λ��
			pRxPacket ++;				//���ݰ���λ������
			if (pRxPacket >= 15)		//����չ�10������
			{
				RxState = 0;			//״̬��0
				Serial_RxFlag = 1;		//�������ݰ���־λ��1���ɹ�����һ�����ݰ�
			}
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		//�����־λ
	}
}
//����openmv����������
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
