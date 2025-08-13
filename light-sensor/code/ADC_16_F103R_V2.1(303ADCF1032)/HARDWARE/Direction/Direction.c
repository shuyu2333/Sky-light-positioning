#include "direction.h"
#include "adc.h"
#include "usart.h"
#include "delay.h"

// 声明外部变量
extern __IO u16 ADC_ConvertedValue[Sample_Num][Channel_Num];

// 存储处理后的朝向结果
static uint16_t processed_direction = 0;

/**
 * @brief  初始化方向处理模块
 * @param  period_ms: 处理周期（毫秒）
 * @retval None
 */
void Direction_Process_Init(uint16_t period_ms) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置定时器
    TIM_TimeBaseStructure.TIM_Period = period_ms * 10 - 1; // 设置周期(10KHz计数频率)
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1; // 72MHz/7200 = 10kHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 使能更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  计算光照方向
 * @param  None
 * @retval 方向角度值（0-359度）
 */
uint16_t Calculate_Direction(void) {
    uint16_t direction = 0;
    uint32_t channel_sums[Channel_Num] = {0};
    
    // 计算每个通道的平均值
    for(int ch = 0; ch < Channel_Num; ch++) {
        for(int sample = 0; sample < Sample_Num; sample++) {
            channel_sums[ch] += ADC_ConvertedValue[sample][ch];
        }
        channel_sums[ch] /= Sample_Num;
    }
    
    // 简单实现：找到信号最强的通道
    uint32_t max_value = 0;
    uint8_t max_channel = 0;
    
    for(int ch = 0; ch < Channel_Num; ch++) {
        if(channel_sums[ch] > max_value) {
            max_value = channel_sums[ch];
            max_channel = ch;
        }
    }
    
    // 将通道号转换为角度（假设传感器均匀分布在圆周上）
    direction = max_channel * (360 / Channel_Num);
    
    return direction;
}

/**
 * @brief  处理传感器数据并输出方向
 * @param  None
 * @retval None
 */
void Process_Sensor_Data(void) {
    // 获取朝向结果
    processed_direction = Calculate_Direction();
    
    // 输出朝向结果
    UART1_SendString("Direction: ");
    
    // 显示朝向角度
    u16 direction = processed_direction;
    u8 hundreds = direction / 100;
    u8 tens = (direction / 10) % 10;
    u8 units = direction % 10;
    
    if(hundreds > 0) {
        UART1_SendByte(hundreds + '0');
    }
    if(hundreds > 0 || tens > 0) {
        UART1_SendByte(tens + '0');
    }
    UART1_SendByte(units + '0');
    
    UART1_SendString(" degrees\n");
}

/**
 * @brief  TIM3中断服务函数
 * @param  None
 * @retval None
 */
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        // 处理传感器数据并计算朝向
        Process_Sensor_Data();
    }
}