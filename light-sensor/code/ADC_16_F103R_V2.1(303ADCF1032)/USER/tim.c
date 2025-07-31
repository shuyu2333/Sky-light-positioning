// tim.c
#include "stm32f10x_tim.h"

#define TIM_TIMEBASE TIM2  // 使用TIM2作为时间基准
#define TIM_CLOCK_FREQ 72000000  // 假设系统时钟72MHz
#define TIM_PRESCALER (TIM_CLOCK_FREQ / 1000000 - 1) // 1μs分辨率

void TIM_Timebase_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    // 1. 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 2. 配置定时器基础参数
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF;  // 32位最大值
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    // 3. 初始化定时器
    TIM_TimeBaseInit(TIM_TIMEBASE, &TIM_TimeBaseStructure);
    
    // 4. 启动定时器
    TIM_Cmd(TIM_TIMEBASE, ENABLE);
}