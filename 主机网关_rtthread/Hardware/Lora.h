#ifndef __LORA__
#define __LORA__

#include <stdint.h>

extern float MQ7_PPM,MQ2_PPM,FR;
extern unsigned int wendu,shidu;
extern unsigned int GT,Pass_status;

void Seria3_Init(uint32_t Baud);
void Seria3_SendByte(uint8_t Byte);
void Seria3_SendArray(uint8_t *Array, uint16_t Length);
void Seria3_SendString(char *String);
uint32_t Seria3_Pow(uint32_t X, uint32_t Y);
void Seria3_SendNumber(uint32_t Number, uint8_t Length);
void usart3_receive_callback(uint8_t Byte);

#endif
