#include "timestamp.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"

// 全局变量，记录定时器溢出次数
volatile uint32_t timer_overflow = 0;

void Timestamp_Init(void)
{
    // 声明所有局部变量
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    // 1. 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 2. 配置定时器参数
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF;  // 32位最大值
    TIM_TimeBaseStructure.TIM_Prescaler = 71;         // 72MHz/(71+1)=1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 3. 配置NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 4. 启用中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // 5. 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

uint32_t Timestamp_Get(void)
{
    uint32_t counter;
    uint32_t overflow;
    
    // 确保原子读取
    do {
        overflow = timer_overflow;
        counter = TIM_GetCounter(TIM2);
    } while (overflow != timer_overflow);
    
    // 合成64位时间戳（返回低32位）
    return (uint32_t)(((uint64_t)overflow << 32) | counter);
}

uint32_t Timestamp_Elapsed(uint32_t prev)
{
    uint32_t current = Timestamp_Get();
    
    // 处理计数器回绕
    if (current >= prev) {
        return current - prev;
    } else {
        return (0xFFFFFFFF - prev) + current + 1;
    }
}