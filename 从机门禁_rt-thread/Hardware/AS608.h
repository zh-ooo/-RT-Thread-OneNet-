#ifndef __AS608_H
#define __AS608_H

void Serial_Init(uint32_t Baud);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
extern uint8_t Serial_RxFlag;	
extern uint8_t Serial_RxPacket[15];
extern uint8_t Serial_TxPacket[12];
extern uint8_t Face_status;

#endif
