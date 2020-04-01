/**
  ******************************************************************************
  * @file    SoftTmr.c
  * @author  Hony
  * @version V1.0
  * @date    2016-12-12
  * @brief   软件定时器
  @verbatim
   2016-12-12 V1.0
   创建
   2019-01-09 V1.1
   修改定时器实现，采用升序单向链表，每次只用比对第一个是否到期即可；
   修改定时器中断回调函数，增加主循环运行函数，将大部分处理放在主循环中；
  @endverbatim
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************  
  */  
#include "softtmr.h"


static Tmr_TCB Tmr_Buff[TMR_MAX_LEN] = {0};
static Tmr_TCB *pTmrUsed;//已使用的定时器内存池的表头
static Tmr_TCB *pTmrIdle;//未使用的定时器内存池的表头
static volatile unsigned int tmr_tick = 0;
static volatile unsigned char tmr_cnt = 0;

/**
  * @brief  软件定时的初始化
  * @param  无
  * @retval 无
  */
void Tmr_Init(void)
{
    unsigned char i;
    pTmrUsed = NULL;
    pTmrIdle = Tmr_Buff;
    for(i=0;i<(TMR_MAX_LEN - 1);i++)
    {
        Tmr_Buff[i].pNext = &Tmr_Buff[i+1];
    } 
    Tmr_Buff[i].pNext = NULL;
	tmr_tick = 0;
    tmr_cnt = 0;
}


/**
  * @brief  获取一个数据结构
  * @param  无
  * @retval Tmr_TCB *
  */
Tmr_TCB * Tmr_Malloc(void)
{
    Tmr_TCB * pList = NULL;
    if(pTmrIdle != NULL)
    {
        pList = pTmrIdle;
        pTmrIdle = pTmrIdle->pNext;
        pList->pNext = NULL;
    }
    return pList;
}

/**
  * @brief  释放一个数据结构
  * @param  pList 
  * @retval 无
  */
void Tmr_Free(Tmr_TCB * pList)
{
    if(pList != NULL)
    {
        pList->pNext = pTmrIdle;//插入表头
        pTmrIdle = pList;
    }
}

/**
  * @brief  删除定时器
  * @param  Id 定时器ID
  * @retval Tmr_Result
  */
Tmr_Result Tmr_Del(unsigned char Id)
{
    Tmr_Result res = TMR_ERR;
    Tmr_TCB *pList = pTmrUsed;
    Tmr_TCB *pPrev = NULL;
    while(pList != NULL)//找到相同的删除
    {
        if(pList->Info.Id == Id)
        {
            if(pPrev != NULL)
            {
                pPrev->pNext = pList->pNext;
            }
            else
            {
                pTmrUsed = pTmrUsed->pNext;
            }
            Tmr_Free(pList);
            res = TMR_OK;
            tmr_cnt--;
            break;
        }  
        pPrev = pList;
        pList = pList->pNext;
    }
    return res;
}

/**
  * @brief  查找所有定时器中是否含有指定定时器
  * @param  Id 定时器ID
  * @retval Tmr_Result
  */
Tmr_Result Tmr_Find(unsigned char Id)
{
    Tmr_Result res = TMR_ERR;
    Tmr_TCB *pList = pTmrUsed;
    while(pList != NULL)//找到相同的删除
    {
        if(pList->Info.Id == Id)
        {
            res = TMR_OK;
            break;
        }  
        pList = pList->pNext;
    }
    return res;
}

/**
  * @brief  创建定时器，如果ID存在，则相当于重装定时器
  * @param  Id 定时器ID
  * @param  Period 定时周期 /100ms
  * @param  Mode  定时模式，TMR_MODE_PERIODIC 重复运行，TMR_MODE_ONE_SHOT 运行一次
  * @param  Callback 定时到期回调函数
  * @retval Tmr_Result
  */
Tmr_Result Tmr_Add(unsigned char Id, unsigned int Period, Tmr_Run_Mode Mode, _cbTmOt Callback)
{
    Tmr_Result res;
    Tmr_TCB *pRev = NULL;
    Tmr_TCB *pNew;
    Tmr_TCB *pList;
    Tmr_Del(Id);//删除相同的ID,确保链表中没有相同的ID
    pList = pTmrUsed;
    pNew = Tmr_Malloc();//申请空间
    if(pNew != NULL)
    {
        pNew->pNext = NULL;
        pNew->Info.Callback = Callback;
        pNew->Info.Id = Id;
        pNew->Info.Period = Period;
        pNew->Info.RunMode = Mode;
        pNew->Info.Tick = tmr_tick + (pNew->Info.Period * (TMR_TICK_FREQ/TMR_FREQ));
        if(pList != NULL)
        {
            while(pList != NULL)
            {
                if(pNew->Info.Tick  <= pList->Info.Tick)
                {
                    if(pRev == NULL)//第一个位置
                    {//插入表头
                        pNew->pNext = pTmrUsed;
                        pTmrUsed = pNew;
                    }
                    else//中间位置
                    {//插入表中
                        pNew->pNext = pList;
                        pRev->pNext = pNew;
                    }
                    break;
                }
                else if(pList->pNext == NULL)//到最后了
                {//插入表尾部
                    pList->pNext = pNew;
                    break;
                }
                pRev = pList;
                pList = pList->pNext;     
            }     
        }
        else
        {
            pTmrUsed = pNew;
        }
        res = TMR_OK;
        tmr_cnt++;
    }
    else
    {
        res = TMR_ERR;
    }
    return res;
}


/**
  * @brief  复位定时器，没有此定时器，退出
  * @param  Id 定时器ID
  * @retval Tmr_Result
  */
Tmr_Result Tmr_Reset(unsigned char Id)
{
    Tmr_Result res = TMR_ERR;
    Tmr_TCB *pTemp = pTmrUsed;
    while(pTemp != NULL)//找到相同的ID
    {
        if(pTemp->Info.Id == Id)
        {
            res = TMR_OK;
            break;
        }  
        pTemp = pTemp->pNext;
    }
    if(res == TMR_OK)
    {
        Tmr_Add(pTemp->Info.Id, pTemp->Info.Period, pTemp->Info.RunMode, pTemp->Info.Callback);
    }
    return res;
}


/**
  * @brief  定时器中断回调函数，需要被周期性调用
  * @param  无
  * @retval 无
  */
void Tmr_IRQCallback(void)
{
	tmr_tick++;
}


/**
  * @brief  定时器运行
  * @param  无
  * @retval 无
  */
void Tmr_Run(void)
{
    Tmr_TCB *pTemp = NULL;
    _cbTmOt _cb = NULL;
    while(pTmrUsed != NULL)
    {
        pTemp = pTmrUsed;
        if(tmr_tick >= pTemp->Info.Tick)
		{
            _cb = pTemp->Info.Callback;
            if(pTemp->Info.RunMode == TMR_MODE_PERIODIC)
            {
                Tmr_Reset(pTemp->Info.Id);
            }
            else
            {
                Tmr_Del(pTemp->Info.Id);
            }
            if(_cb != NULL)
            {
                _cb();
            }
        }
        else
        {
            break;
        }
    }
}



