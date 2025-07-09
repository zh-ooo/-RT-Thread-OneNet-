#ifndef __serial__
#define __serial__

#define RX1_BUFFER_SIZE 256

#include <stdint.h>

typedef struct {
	uint8_t RxBuffer[RX1_BUFFER_SIZE];
	uint16_t RxDataCnt;
	uint8_t RxData;
}UART_TypeDef;

void Serial_Init(uint32_t Baud);
void Serial_SendByte(uint8_t Byte);
void usart1_receive_callback(uint8_t Byte);

void uart2_init(uint32_t baud);
void uart2_send_byte(uint8_t byte);
void uart2_send_string(char *str);


#endif
