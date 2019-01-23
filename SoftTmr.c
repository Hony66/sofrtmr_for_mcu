/**
  ******************************************************************************
  * @file    SoftTmr.c
  * @author  Hony
  * @version V1.1
  * @date    2016-12-12
  * @brief   软件定时器
  @verbatim
         最多可以使用TMR_MAX_LEN个定时器
  @endverbatim
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************  
  */
  
#include "softtmr.h"


static Tmr_TCB Tmr_Buff[TMR_MAX_LEN] = {0};
static Tmr_TCB *pTmrUsed;
static Tmr_TCB *pTmrIdle;


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
                if(pList->pNext != NULL)
                {
                    pList->pNext->Info.Tick += pList->Info.Tick;
                }
            }
            else
            {
                if(pTmrUsed->pNext != NULL)
                {
                    pTmrUsed->pNext->Info.Tick += pTmrUsed->Info.Tick;
                }
                pTmrUsed = pTmrUsed->pNext;
            }
            Tmr_Free(pList);
            res = TMR_OK;
            break;
        }  
        pPrev = pList;
        pList = pList->pNext;
    }
    return res;
}


/**
  * @brief  创建定时器，如果ID存在，则相当于重装定时器
  * @param  Id 定时器ID
  * @param  Period 定时周期 /100ms
  * @param  Mode  定时模式，MULTI_SHOOT 重复运行，ONE_SHOOT 运行一次
  * @param  Callback 定时到期回调函数
  * @retval Tmr_Result
  */
Tmr_Result Tmr_Add(unsigned char Id, unsigned short int Period, Tmr_Run_Mode Mode, _cbTmOt Callback)
{
    unsigned short int tTick = Period;
    Tmr_Result res;
    Tmr_TCB *pPrev;
    Tmr_TCB *pNew;
    Tmr_TCB *pList;
    Tmr_Del(Id);//删除相同的ID,确保链表中没有相同的ID
    pList = pTmrUsed;
    pNew = Tmr_Malloc();//申请空间
    pPrev = NULL;
    if(pNew != NULL)
    {
        pNew->pNext = NULL;
        pNew->Info.Callback = Callback;
        pNew->Info.Id = Id;
        pNew->Info.Period = Period;
        pNew->Info.RunMode = Mode;
        if(pList != NULL)
        {
            while(pList != NULL)
            {
                if(tTick <= pList->Info.Tick)
                {
                    if(pPrev != NULL)
                    {
                        pPrev->pNext = pNew;
                    }
                    pNew->pNext = pList;
                    pList->Info.Tick -= tTick;
                    pNew->Info.Tick = tTick;
                    if(pList == pTmrUsed)
                    {
                        pTmrUsed = pNew;
                    }
                    break;
                }
                tTick -= pList->Info.Tick;
                if(pList->pNext == NULL)
                {
                    pList->pNext = pNew;
                    pNew->pNext = NULL;
                    pNew->Info.Tick = tTick;
                    break;
                }
                pPrev = pList;
                pList = pList->pNext;  
                
            }     
        }
        else
        {
            pTmrUsed = pNew;
            pTmrUsed->Info.Tick = tTick;
        }
        res = TMR_OK;
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
        res = Tmr_Add(pTemp->Info.Id, pTemp->Info.Period, pTemp->Info.RunMode, pTemp->Info.Callback);
    }
    return res;
}



/**
  * @brief  定时器中断回调函数，需要被周期性调用 当前10hz
  * @param  无
  * @retval 无
  */
void Tmr_IRQCallback(void)
{
    if(pTmrUsed != NULL)
    {
        while(pTmrUsed->Info.Tick == 0)
        {
            if(pTmrUsed->Info.Callback != NULL)
            {
                pTmrUsed->Info.Callback();
            }
            if(pTmrUsed->Info.RunMode == MULTI_SHOOT)
            {
                Tmr_Reset(pTmrUsed->Info.Id);
            }
            else
            {
                Tmr_Del(pTmrUsed->Info.Id);
            }
        }
        if(pTmrUsed != NULL);
        {
            pTmrUsed->Info.Tick--;
        }
    }
}








