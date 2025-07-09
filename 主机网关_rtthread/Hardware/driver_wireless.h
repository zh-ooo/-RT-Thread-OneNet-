#ifndef __driver_wireless__
#define __driver_wireless__

#include <stdint.h>

#define W_ENABLE 	1
#define W_DISABLE 	0

#define W_FALSE 	0
#define W_TRUE 		1

#define W_OK		1    

#define ERROR_WiFi_CONNECT	(0x01 << 15)
#define ERROR_MQTT_CONNECT	(0x01 << 14)
#define ERROR_PUBLISH		(0x01 << 13)

// 定义类型标识常量
#define TYPE_BOOL		(0x01 << 0)
#define TYPE_INT 		(0x01 << 1)
#define TYPE_FLOAT 		(0x01 << 2)
#define TYPE_STRING 	(0x01 << 3)

typedef struct {
    uint8_t receiveDataFlag;		//接收数据标志位
    uint16_t error_code;			//无线模块初始化错误代码
}Wireless_TypeDef;

// 定义联合体，用于存储不同类型的数据
typedef union {
	uint8_t	bool_value;      //布尔型
    int16_t int_value;       // 整型
    float float_value;       // 浮点型
    const char *string_value; // 字符串
} Type_Value;

typedef struct 
{
	char *name;
	uint8_t type;
	Type_Value value;
}OneNET_MQTT_Data;

extern OneNET_MQTT_Data *onenet_data_array[];
extern Wireless_TypeDef WirelessStatus;

void wireless_clear_buffer(void);
uint8_t wireless_mqtt_connect_status(void);
uint8_t wireless_wifi_connect_status(void);
uint8_t wireless_get_receive_flag(void);
uint8_t wireless_send_command(char *cmd, char *res, uint8_t sendCount, uint8_t clear_buffer, uint16_t delay_xms, uint8_t printf_enable);
void wireless_init(void);
void wireless_error_handler(uint16_t error_code);
void wireless_system_handler(void);
void wireless_onenet_data_handler(void);
void wireless_publish_data(void);
uint8_t wireless_get_onenet_command_flag(void);
uint32_t wireless_get_command_id(char *jsonData);
void wireless_receiver_command_extract(char *jsonData);
void wireless_receive_command_respond(uint32_t respond_id, uint16_t code, char *msg);
void wireless_command_control(void);
void wireless_processing_command(void);
void wireless_receive_data_handler(void);
void wireless_receive_callback(uint8_t byte);



#endif
