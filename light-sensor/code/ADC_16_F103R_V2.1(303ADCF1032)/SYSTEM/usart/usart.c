#include "sys.h"
#include "usart.h"	  
#include "DMA.h"

#if SYSTEM_SUPPORT_OS
#include "includes.h"					
#endif

#if 1
#pragma import(__use_no_semihosting)             
/* 标准库需要的支持函数 */                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
/* 定义_sys_exit()以避免使用半主机模式 */    
void _sys_exit(int x) 
{ 
	x = x; 
} 

/* 重定向printf到USART1 */
int fputc(int ch, FILE *f)
{      
	/* 循环发送，直到发送完毕 */
	while((USART1->SR & 0X40) == 0);
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#if EN_USART1_RX   /* 如果使能接收 */
/* 串口1中断服务程序 */
u8 USART_RX_BUF[USART_REC_LEN];     /* 接收缓冲，最大USART_REC_LEN个字节 */
/* 接收状态 */
/* bit15: 接收完成标志 */
/* bit14: 接收到0x0d */
/* bit13~0: 接收到的有效字节数目 */
u16 USART_RX_STA = 0;       /* 接收状态标志 */

#define BUF_SIZE 128
uint8_t adc_buf_ready[BUF_SIZE]; /* 就绪数据 */
uint8_t adc_buf_active[BUF_SIZE]; /* 发送中数据 */
int buf_lock = 0;   /* 缓冲区切换锁 */
extern volatile uint8_t dma_tx_complete;

void uart_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); /* 使能USART1和GPIOA时钟 */
  
    /* USART1_TX   GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; /* PA.9 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; /* 复用推挽输出 */
    GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIOA.9 */
     
    /* USART1_RX   GPIOA.10初始化 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; /* PA10 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 浮空输入 */
    GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIOA.10 */  

    /* Usart1 NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; /* 抢占优先级3 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; /* 子优先级3 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; /* IRQ通道使能 */
    NVIC_Init(&NVIC_InitStructure); /* 初始化NVIC */

    /* USART 初始化设置 */
    USART_InitStructure.USART_BaudRate = bound; /* 串口波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; /* 字长为8位数据格式 */
    USART_InitStructure.USART_StopBits = USART_StopBits_1; /* 一个停止位 */
    USART_InitStructure.USART_Parity = USART_Parity_No; /* 无奇偶校验位 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件数据流控制 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; /* 收发模式 */

    USART_Init(USART1, &USART_InitStructure); /* 初始化串口1 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); /* 开启串口接收中断 */
    USART_Cmd(USART1, ENABLE); /* 使能串口1 */
}
void UART1_DMA_Send(uint8_t *data, uint16_t len) 
{
    DMA_InitTypeDef DMA_InitStructure;
    
    /* 等待上一次发送完成 */
    while (!dma_tx_complete);
    
    /* 禁用DMA通道 */
    DMA_Cmd(DMA1_Channel4, DISABLE);
    
    /* 使用标准库函数设置DMA */
    
    /* 配置DMA */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR); /* 外设地址 */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data; /* 内存地址 */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; /* 方向：内存到外设 */
    DMA_InitStructure.DMA_BufferSize = len; /* 数据长度 */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /* 外设地址不递增 */
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; /* 内存地址递增 */
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* 外设数据宽度：字节 */
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /* 内存数据宽度：字节 */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; /* 正常模式 */
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; /* 中等优先级 */
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; /* 非内存到内存模式 */
    
    DMA_Init(DMA1_Channel4, &DMA_InitStructure); /* 初始化DMA */
    
    dma_tx_complete = 0;
    DMA_Cmd(DMA1_Channel4, ENABLE); /* 使能DMA通道 */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); /* 使能USART的DMA发送请求 */
}

/* 发送一个字节 */
void UART1_SendByte(u8 data)
{
    static uint8_t byte_buf;
    byte_buf = data;
    
    // 等待上次DMA传输完成
    while(!dma_tx_complete) {
        __WFI();
    }
    
    // 使用DMA发送
    UART1_DMA_Send(&byte_buf, 1);
}

/* 发送字符串 */
void UART1_SendString(u8 *s)
{
    /* 逐个发送字符 */
    while(*s)
    {
        UART1_SendByte(*s++);
    }
}

/* 使用DMA发送数据 */


/* DMA发送触发器 */
void UART1_DMA_Send_Trigger(void) 
{
    DMA_InitTypeDef DMA_InitStructure;
    
    /* 检查DMA是否空闲（通过检查DMA通道是否使能） */
    if ((DMA1_Channel4->CCR & 0x1) == 0) 
    { 
        /* 切换缓冲区 */
        memcpy(adc_buf_active, adc_buf_ready, BUF_SIZE);
        
        /* 使用标准库函数重新配置DMA */
        
        /* 配置DMA */
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buf_active;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_InitStructure.DMA_BufferSize = BUF_SIZE;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        
        DMA_Init(DMA1_Channel4, &DMA_InitStructure);
        
        DMA_Cmd(DMA1_Channel4, ENABLE);
        USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); /* 启动传输 */
    }
}

/* USART1中断服务函数 */
void USART1_IRQHandler(void)
{
    u8 Res;
#if SYSTEM_SUPPORT_OS /* 如果SYSTEM_SUPPORT_OS为真，表示支持OS */
    OSIntEnter();    
#endif
    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  /* 接收中断(接收到的数据必须是0x0d 0x0a结尾) */
    {
        Res = USART_ReceiveData(USART1); /* 读取接收到的数据 */
        
        if((USART_RX_STA & 0x8000) == 0) /* 接收未完成 */
        {
            if(USART_RX_STA & 0x4000) /* 接收到了0x0d */
            {
                if(Res != 0x0a) 
                    USART_RX_STA = 0; /* 接收错误，重新开始 */
                else 
                    USART_RX_STA |= 0x8000; /* 接收完成了 */
            }
            else /* 还没收到0X0D */
            {	
                if(Res == 0x0d) 
                    USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
                    USART_RX_STA++;
                    if(USART_RX_STA > (USART_REC_LEN - 1)) 
                        USART_RX_STA = 0; /* 接收数据错误，重新开始接收 */
                }		 
            }
        }   		 
    } 
    
#if SYSTEM_SUPPORT_OS /* 如果SYSTEM_SUPPORT_OS为真，表示支持OS */
    OSIntExit();  											 
#endif
} 
#endif