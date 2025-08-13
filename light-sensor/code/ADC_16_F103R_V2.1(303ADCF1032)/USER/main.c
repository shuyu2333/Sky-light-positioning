#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_rcc.h"
#include "delay.h"
#include "usart.h"
#include "adc.h"
#include "DMA.h"
#include "direction.h"  // 添加方向处理模块头文件

extern __IO uint32_t adc_timestamp;
uint8_t adc_data_buffer[128]; 

void Display_Adc_Val(u16 Adc_Val);
void Display_Adc_Vol(u16 Adc_Vol);
void Display_TimeStamp(u32 timestamp);
void Display_ADC(void);
void Display_ADC_DMA(void);

// 定时器中断服务函数
void TIM2_IRQHandler(void);


void Display_Adc_Val(u16 Adc_Val)      
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

 
void Display_ADC_DMA(void) {
    static uint32_t last_send_time = 0;
    uint32_t current_time = get_timestamp_ms();
    
    // 控制发送频率（1ms间隔）
    if (current_time - last_send_time >= 1) {
        int len = sprintf((char*)adc_data_buffer, "1 %lu ...", adc_timestamp); // 格式化数据
        UART1_DMA_Send(adc_data_buffer, len); // 非阻塞发送
        last_send_time = current_time;
    }
}
void TIM2_IRQHandler(void) {  // 定时器中断服务函数
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        // 原有的数据显示处理可以保留或移除，根据需要决定
        // Display_ADC_DMA_Handler();  // 如果不需要可以注释掉
    }
}


int main(void)
{	
    SystemInit();	          // 系统初始化
    delay_init();	    	  // 延时函数初始化	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置NVIC优先级分组为2:2位抢占优先级，2位响应优先级
    uart_init(115200);        // 串口初始化波特率为115200
    Adc_Init();               // ADC和DMA初始化
    TIM_Config(1);            // 定时器周期1ms
    TIM3_Config(10);         // 配置TIM3，10ms处理一次数据
    
    while(1)
    {
        __WFI(); // CPU休眠，等待中断唤醒
    }
}
	