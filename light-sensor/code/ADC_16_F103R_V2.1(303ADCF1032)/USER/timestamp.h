// timestamp.h
#ifndef __TIMESTAMP_H
#define __TIMESTAMP_H

#include <stdint.h>

// 初始化时间戳系统
void Timestamp_Init(void);

// 获取当前时间戳（微秒）
uint32_t Timestamp_Get(void);

// 计算时间间隔（微秒）
uint32_t Timestamp_Elapsed(uint32_t prev);

#endif