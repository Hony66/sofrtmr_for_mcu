#ifndef __SOFTTMR_H
#define __SOFTTMR_H

//软件定时器tick运行频率
#define TMR_TICK_FREQ       1000U//HZ  
//运行单位(例如10HZ相当于100ms), 如果赋给定时器的 Period = 10, 那么实际的定时时间应该是Period * (TMR_TICK_FREQ/TMR_FREQ)  ms
#define TMR_FREQ            10U//HZ 相当于100ms



#define TMR_MAX_LEN         16U

#ifndef NULL
    #define NULL 0u
#endif


typedef enum {
    TMR_OK = 0,
    TMR_ERR,
}Tmr_Result;


typedef enum {
    TMR_MODE_ONE_SHOT = 0,
    TMR_MODE_PERIODIC,
}Tmr_Run_Mode;


typedef void(*_cbTmOt)(void);

typedef struct{
    volatile Tmr_Run_Mode    RunMode;       // Timer运行模式
    volatile unsigned char   Id;            // Timer ID号
    volatile unsigned int    Period;        // 定时时间
    volatile unsigned int    Tick;          // 计数器
    volatile _cbTmOt Callback;
}Tmr_Info;



typedef struct TMR_TCB_STRUCT {
    struct TMR_TCB_STRUCT *pNext; 
    Tmr_Info Info;
}Tmr_TCB;



void Tmr_Init(void);
Tmr_Result Tmr_Add(unsigned char Id, unsigned int Period, Tmr_Run_Mode Mode, _cbTmOt Callback);
Tmr_Result Tmr_Del(unsigned char Id);
Tmr_Result Tmr_Reset(unsigned char Id);
void Tmr_IRQCallback(void);
Tmr_Result Tmr_Find(unsigned char Id);
void Tmr_Run(void);















#endif 



