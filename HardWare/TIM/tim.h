#ifndef _TIM_H
#define _TIM_H

#include "stm32f10x.h"


void TIM_Init(u32 arr, u32 psc);
u8 TIM_Set(TIM_TypeDef* TIMx, FunctionalState NewState);

typedef struct{
	u8 hour;
	u8 min;
	u8 sec;
}counter_obj;


 
extern counter_obj counter;
extern u8 counter_value;

#endif 

