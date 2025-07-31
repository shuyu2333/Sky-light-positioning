#include "DMA.h"
#include "adc.h"
#include "delay.h"

__IO uint32_t adc_timestamp = 0;
extern __IO u16 ADC_ConvertedValue[Sample_Num][Channel_Num];        // ADC转换值数组

void MYDMA_Config(void)
{
    // 所有声明必须在函数开头（任何可执行语句之前）
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	// 启用DMA时钟
	
	DMA_DeInit(DMA1_Channel1);   // 复位DMA通道1
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  // 外设地址(ADC数据寄存器)
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;  // 内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // 外设作为数据传输源
	DMA_InitStructure.DMA_BufferSize = Sample_Num * Channel_Num;  // 传输数据量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // 外设地址不自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  // 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  // 外设数据宽度16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // 内存数据宽度16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; // 高优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  // 内存到内存模式禁止
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  // 初始化DMA
	
	// 启用传输完成中断
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    
    // 配置NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
	DMA_Cmd(DMA1_Channel1, ENABLE);                // 启用DMA通道
} 

void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        // 获取当前时间戳（微秒）
        adc_timestamp = get_timestamp_us();
        
        // 清除中断标志
        DMA_ClearITPendingBit(DMA1_IT_TC1);
    }
}