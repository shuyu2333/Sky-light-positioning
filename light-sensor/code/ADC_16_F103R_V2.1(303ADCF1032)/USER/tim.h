#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

// 函数声明
void TIM_Timebase_Init(void);
void TIM_Config(uint16_t period_ms);
void TIM3_Config(uint16_t period_ms);

#endif
