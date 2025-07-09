#ifndef __AD_H
#define __AD_H

extern uint16_t AD_Value[3];

extern float MQ2_PPM;
extern float FR;
extern float MQ7_PPM;

void AD_Init(void);
void AD_GetValue(void);
float MQ7_GetData_PPM(void);
float Fire_GetData(void);
float MQ2_GetData_PPM(void);

#endif