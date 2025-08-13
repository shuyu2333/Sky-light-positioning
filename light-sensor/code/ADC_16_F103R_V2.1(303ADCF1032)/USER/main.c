#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32f10x_rcc.h"
#include "delay.h"
#include "usart.h"
#include "adc.h"
#include "DMA.h"
extern __IO uint32_t adc_timestamp;
uint8_t adc_data_buffer[128]; 

extern __IO u16 ADC_ConvertedValue[Sample_Num][Channel_Num];
uint16_t processed_direction = 0; // 存储处理后的朝向结果

void Process_Sensor_Data(void);
uint16_t Calculate_Direction(void);

void TIM3_Config(uint16_t period_ms);

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

// 在main.c中
void TIM2_IRQHandler(void) {  // 定时器中断服务函数
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Display_ADC_DMA_Handler();  // 触发数据显示
    }
}

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
    
    // 这里只是一个占位符实现
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

// 处理传感器数据的函数
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

// 在main.c中添加TIM3中断处理函数
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        // 处理传感器数据并计算朝向
        Process_Sensor_Data();
    }
}

// 在main.c中添加TIM3配置函数
void TIM3_Config(uint16_t period_ms) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置定时器
    TIM_TimeBaseStructure.TIM_Period = period_ms; // 设置周期
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


int main(void)
{	
    SystemInit();	          // 系统初始化
    delay_init();	    	  // 延时函数初始化	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置NVIC优先级分组为2:2位抢占优先级，2位响应优先级
    uart_init(115200);        // 串口初始化波特率为115200
    Adc_Init();               // ADC和DMA初始化
    TIM_Config(1);            // 定时器周期1ms（根据需求调整）
    TIM3_Config(100);         // 配置TIM3，100ms处理一次数据（可根据需要调整）
    
    while(1)
    {
        __WFI(); // CPU休眠，等待中断唤醒
    }
}
	