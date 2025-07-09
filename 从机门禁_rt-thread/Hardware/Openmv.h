#ifndef __OPENMV_H
#define __OPENMV_H

#include <stdio.h>

extern char Seria2_RxPacket[];
extern uint8_t Seria2_RxFlag;

void Seria2_Init(uint32_t Baud);
void Seria2_SendByte(uint8_t Byte);
void Seria2_SendArray(uint8_t *Array, uint16_t Length);
void Seria2_SendString(char *String);

#endif