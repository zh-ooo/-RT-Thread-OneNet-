#ifndef __LORA__
#define __LORA__

#include <stdint.h>

void Seria3_Init(uint32_t Baud);
void Seria3_SendByte(uint8_t Byte);
void Seria3_SendArray(uint8_t *Array, uint16_t Length);
void Seria3_SendString(char *String);

#endif