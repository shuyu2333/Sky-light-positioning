#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "tim.h"

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

void TIM_Config(uint16_t period_ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 配置定时器基础参数
    TIM_TimeBaseStructure.TIM_Period = period_ms * 1000 - 1;  // 设置周期为指定毫秒数
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    // 初始化定时器
    TIM_TimeBaseInit(TIM_TIMEBASE, &TIM_TimeBaseStructure);
    
    // 使能更新中断
    TIM_ITConfig(TIM_TIMEBASE, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM_TIMEBASE, ENABLE);
}

void TIM3_Config(uint16_t period_ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置定时器基础参数
    TIM_TimeBaseStructure.TIM_Period = period_ms * 1000 - 1;  // 设置周期为指定毫秒数
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    // 初始化定时器
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 使能更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}
