#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
#include "DMA.h"

#define  ADC1_DR_Address    ((u32)0x40012400+0x4c)      //����DMA�������ַ,��Ϊ���ADC1ת������ļĴ���������DAM��������ȡ����

void Adc_Init(void);                                 //ADC1��ʼ��

u16 ReadADCAverageValue(u8 Channel);     //ÿ��ͨ��������ȡ��10�Σ����ƽ��ֵ


 #endif 