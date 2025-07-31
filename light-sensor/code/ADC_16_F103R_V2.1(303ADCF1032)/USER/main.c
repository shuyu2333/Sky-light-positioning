#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_rcc.h"
#include "delay.h"
#include "usart.h"
#include "adc.h"
#include "DMA.h"
extern __IO uint32_t adc_timestamp;


void Display_Adc_Val(u16 Adc_Val)       //Дʾ12λADCֵ
{
	u8 qian,bai,shi,ge;
	qian=Adc_Val/1000;
	bai =Adc_Val/100%10;
	shi =Adc_Val/10%10;
	ge = Adc_Val%10;
	UART1_SendByte(qian+ '0');
	UART1_SendByte(bai + '0');
	UART1_SendByte(shi + '0');
	UART1_SendByte(ge  + '0');
}




void Display_Adc_Vol(u16 Adc_Vol)       
{
	u16 decimal1,decimal2,decimal3;      
	float temp;
	u16 temp1;
	
	temp=(float)Adc_Vol*(3.3/4096);    
	temp1=temp;                      
	UART1_SendByte(temp1+'0');       
	
	temp-=temp1;                   
	temp*=1000;                 
	
	decimal1=(u16)temp/100;      
	decimal2=(u16)temp%100/10;    
	decimal3=(u16)temp%10;        
  
	UART1_SendByte('.');           
	UART1_SendByte(decimal1+'0'); 
	UART1_SendByte(decimal2+'0');  
	UART1_SendByte(decimal3+'0');  
	
}

void Display_TimeStamp(u32 timestamp)    // 添加的时间戳显示函数
{
    u32 time_us = timestamp / 1;      // 转换为 微秒
    u8 digits[10] = {0};
    int i;
    
    // 提取时间戳的每一位数字
    for (i = 9; i >= 0; i--) {
        digits[i] = time_us % 10;
        time_us /= 10;
    }
    
    // 输出时间戳
    for (i = 0; i <= 9; i++) {
        UART1_SendByte(digits[i] + '0');
    }
}


void Display_ADC(void)          
{
	UART1_SendString("1 "); 
    Display_TimeStamp(adc_timestamp);
	Display_Adc_Val(ReadADCAverageValue(0));
	Display_Adc_Val(ReadADCAverageValue(1));
	Display_Adc_Val(ReadADCAverageValue(2));
	Display_Adc_Val(ReadADCAverageValue(3));
	Display_Adc_Val(ReadADCAverageValue(4));
	Display_Adc_Val(ReadADCAverageValue(5));
	Display_Adc_Val(ReadADCAverageValue(6));
	Display_Adc_Val(ReadADCAverageValue(7));
	Display_Adc_Val(ReadADCAverageValue(8));
	Display_Adc_Val(ReadADCAverageValue(9));
	Display_Adc_Val(ReadADCAverageValue(10));
	Display_Adc_Val(ReadADCAverageValue(11));
	Display_Adc_Val(ReadADCAverageValue(12));
	Display_Adc_Val(ReadADCAverageValue(13));
	Display_Adc_Val(ReadADCAverageValue(14));
	Display_Adc_Val(ReadADCAverageValue(15));
	UART1_SendString(" 0"); 
	UART1_SendString("\n"); 
			
	
}

 

 
 int main(void)
 {	


	SystemInit();	      //ϵͳԵʼۯ
	delay_init();	    	//ғʱگ˽Եʼۯ	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//ʨ׃א׏ԅЈܶؖةΪة2ú2λȀռԅЈܶì2λЬӦԅЈܶ
	uart_init(115200);    //ԮࠚԵʼۯìҨ͘Ê115200
	Adc_Init();           //ADCۍDMAԵʼۯ
	 
   
  while(1)
	{
		
		 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);//ֈսԫˤΪԉرղ֚һλ˽ߝɝӗ֪
		 
		Display_ADC();                                           //Дʾ12λADCֵۍ֧ѹֵ
		 UART1_SendString("\r\n");                               
		 delay_ms(1);                                           
		
	}
	
 }