#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
#include "DMA.h"

#define  ADC1_DR_Address    ((u32)0x40012400+0x4c)      //定义DMA外设基地址,即为存放ADC1转换结果的寄存器，告诉DAM到这里来取数据

void Adc_Init(void);                                 //ADC1初始化

u16 ReadADCAverageValue(u8 Channel);     //每个通道的数据取样10次，求得平均值


 #endif 