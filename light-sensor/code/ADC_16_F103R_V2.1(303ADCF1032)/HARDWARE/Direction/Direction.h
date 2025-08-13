#ifndef __DIRECTION_H
#define __DIRECTION_H

#include "stm32f10x.h"

// 方向处理模块初始化
void Direction_Process_Init(uint16_t period_ms);

// 方向计算函数
uint16_t Calculate_Direction(void);

// 传感器数据处理函数
void Process_Sensor_Data(void);

#endif