#ifndef __DHT11_H
#define __DHT11_H

//DHT11���ź궨��
#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_Pin_11
#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOA
/*********************END**********************/

//���״̬����
#define OUT 1
#define IN  0

//����DHT11��������ߵ͵�ƽ
#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)

extern uint8_t wendu,shidu;

u8 DHT11_Init(void);//��ʼ��DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//��ȡ��ʪ������
u8 DHT11_Read_Byte(void);//��ȡһ���ֽڵ�����
u8 DHT11_Read_Bit(void);//��ȡһλ������
void DHT11_Mode(u8 mode);//DHT11�������ģʽ����
u8 DHT11_Check(void);//���DHT11
void DHT11_Rst(void);//��λDHT11   

#endif