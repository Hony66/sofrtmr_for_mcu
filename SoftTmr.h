#ifndef __SOFTTMR_H
#define __SOFTTMR_H

#define TMR_FREQ        10U//HZ, 这个参数需要根据调用Tmr_IRQCallback的频率填写
#define TMR_MAX_LEN     10U

#ifndef NULL
    #define NULL 0u
#endif


typedef enum {
    TMR_OK = 0,
    TMR_ERR,
}Tmr_Result;


typedef enum {
    ONE_SHOOT = 0,
    MULTI_SHOOT,
}Tmr_Run_Mode;


typedef void(*_cbTmOt)(void);

typedef struct{
    volatile Tmr_Run_Mode   RunMode;       // Timer运行模式，RUN_ALWAYS，RUN_ONCE
    volatile unsigned char   Id;            // Timer ID号
    volatile unsigned short int  Period;        // 定时时间
    volatile unsigned short int  Tick;          // 计数器
    volatile _cbTmOt Callback;
}Tmr_Info;



typedef struct TMR_TCB_STRUCT {
    struct TMR_TCB_STRUCT *pNext; 
    Tmr_Info Info;
}Tmr_TCB;



void Tmr_Init(void);
Tmr_Result Tmr_Add(unsigned char Id, unsigned short int Period, Tmr_Run_Mode Mode, _cbTmOt Callback);
Tmr_Result Tmr_Del(unsigned char Id);
Tmr_Result Tmr_Reset(unsigned char Id);
void Tmr_IRQCallback(void);















#endif 



