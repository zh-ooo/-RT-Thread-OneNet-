#ifndef __STEP_H
#define __STEP_H

void Stepper_Init(void);
void Stepper1_Run(uint8_t dir,uint32_t num,uint32_t speed);
void Stepper2_Run(uint8_t dir,uint32_t num,uint32_t speed);

#endif
