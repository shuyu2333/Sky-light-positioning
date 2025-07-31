#include "delay.h"

////////////////////////////////////////////////////////////////////////////////// 
// 注意：本文件同时支持操作系统(OS)和非操作系统环境
//////////////////////////////////////////////////////////////////////////////////  

static u8  fac_us = 0;         // us延时倍乘数
static u16 fac_ms = 0;         // ms延时倍乘数
static volatile uint32_t timestamp_us = 0; // 全局微秒级时间戳

// 如果支持操作系统(OS)
#if SYSTEM_SUPPORT_OS
#include "includes.h" // 包含OS头文件

// OS相关宏定义保持不变
#ifdef OS_CRITICAL_METHOD
#define delay_osrunning       OSRunning
#define delay_ostickspersec    OS_TICKS_PER_SEC
#define delay_osintnesting     OSIntNesting
#endif

#ifdef CPU_CFG_CRITICAL_METHOD
#define delay_osrunning       OSRunning
#define delay_ostickspersec    OSCfg_TickRate_Hz
#define delay_osintnesting     OSIntNestingCtr
#endif

// OS环境下的Scheduler操作函数
void delay_osschedlock(void)
{
    #ifdef CPU_CFG_CRITICAL_METHOD
        OS_ERR err; 
        OSSchedLock(&err);
    #else
        OSSchedLock();
    #endif
}

void delay_osschedunlock(void)
{	
    #ifdef CPU_CFG_CRITICAL_METHOD
        OS_ERR err; 
        OSSchedUnlock(&err);
    #else
        OSSchedUnlock();
    #endif
}

// OS环境下的延时函数
void delay_ostimedly(u32 ticks)
{
    #ifdef CPU_CFG_CRITICAL_METHOD
        OS_ERR err; 
        OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err);
    #else
        OSTimeDly(ticks);
    #endif 
}

// OS环境下的SysTick中断处理
void SysTick_Handler(void)
{	
    if(delay_osrunning == 1)
    {
        OSIntEnter();
        OSTimeTick();
        OSIntExit();
    }
    #if !SYSTEM_SUPPORT_OS
    // 非OS环境下更新时间戳
    else {
        timestamp_us++;
    }
    #endif
}
#endif // SYSTEM_SUPPORT_OS

// 延时初始化函数
void delay_init()
{
    // 配置SysTick时钟源为HCLK/8
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    
    // 计算计数器基本单位（每微秒的计数值）
    fac_us = SystemCoreClock / 8000000;
    
    // 如果支持OS
    #if SYSTEM_SUPPORT_OS
        u32 reload;
        reload = SystemCoreClock / 8000000;
        reload *= 1000000 / delay_ostickspersec;
        fac_ms = 1000 / delay_ostickspersec;

        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
        SysTick->LOAD = reload;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    #else
        // 非OS环境下的配置
        fac_ms = (u16)fac_us * 1000;
        // 配置SysTick为1us中断
        SysTick->LOAD = fac_us;         // 重载值 = 1us计数
        SysTick->VAL = 0;               // 清空当前值
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk |   // 启用中断
                         SysTick_CTRL_ENABLE_Msk;    // 启用计数器
    #endif
}

// OS环境下的延时函数
#if SYSTEM_SUPPORT_OS
void delay_us(u32 nus)
{		
    u32 ticks;
    u32 told, tnow, tcnt = 0;
    u32 reload = SysTick->LOAD;
    
    ticks = nus * fac_us;
    tcnt = 0;
    delay_osschedlock();
    told = SysTick->VAL;
    
    while(1)
    {
        tnow = SysTick->VAL;
        if(tnow != told)
        {
            if(tnow < told) tcnt += told - tnow;
            else tcnt += reload - tnow + told;
            told = tnow;
            if(tcnt >= ticks) break;
        }
    }
    delay_osschedunlock();
}

void delay_ms(u16 nms)
{	
    if(delay_osrunning && delay_osintnesting == 0)
    {
        if(nms >= fac_ms)
        {
            delay_ostimedly(nms / fac_ms);
        }
        nms %= fac_ms;
    }
    delay_us((u32)(nms * 1000));
}
#else // 非OS环境

// 非OS环境下的us延时函数
void delay_us(u32 nus)
{		
    u32 temp;	    	 
    SysTick->LOAD = nus * fac_us;  // 设置重载值
    SysTick->VAL = 0x00;         // 清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 启用计数器
    
    // 等待时间到达
    do {
        temp = SysTick->CTRL;
    } while((temp & 0x01) && !(temp & (1 << 16)));
    
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 禁用计数器
    SysTick->VAL = 0x00;       // 清空计数器
}

// 非OS环境下的ms延时函数
void delay_ms(u16 nms)
{	 		  	  
    u32 temp;
    SysTick->LOAD = (u32)nms * fac_ms; // 设置重载值
    SysTick->VAL = 0x00;              // 清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 启用计数器
    
    // 等待时间到达
    do {
        temp = SysTick->CTRL;
    } while((temp & 0x01) && !(temp & (1 << 16)));
    
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 禁用计数器
    SysTick->VAL = 0x00;       // 清空计数器
}

// 非OS环境下的SysTick中断处理函数
void SysTick_Handler(void)
{
    timestamp_us++; // 每次中断增加1微秒
}
#endif

// 获取当前时间戳（微秒）
uint32_t get_timestamp_us(void)
{
    #if SYSTEM_SUPPORT_OS
        if(delay_osrunning) {
            // OS环境下返回系统时间（这里假设OS每tick代表10ms）
            return delay_ostickspersec * OSTimeGet() * 1000;
        } else {
            return timestamp_us;
        }
    #else
        return timestamp_us;
    #endif
}

// 获取当前时间戳（毫秒）
uint32_t get_timestamp_ms(void)
{
    return get_timestamp_us() / 1000;
}

// 文件末尾空行