#include "tim.h"
#include "usart.h"

counter_obj counter;	//计数器

//set_value:设定的时间，
//返回值：1， 到达预设值，0-未到计时值
u8 Is_TimeOut(u16 set_time)
{
	u8 now_time;
	now_time = counter.hour*38400 + counter.min*60 + counter.sec;
	if(now_time	>= set_time)	//计时达到预设值
	{
		now_time = 0;
		
		return 1;
	}
	return 0;
}


void  TIM_Init(u32 arr,u32 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	 //定时器TIM4初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值    
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
    TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE ); 
	
	//中断设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);  
	
	TIM_Cmd(TIM4,DISABLE);
}


//定时器4中断服务程序
void TIM4_IRQHandler()   //TIM3中断
{   
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 
	}
	
	counter.sec++;	//
	if(counter.sec == 60 || counter.min == 60)
	{
		if(counter.sec == 60)
		{
			counter.sec = 0;
			counter.min++;
		}
		else if(counter.min == 60)
		{

			counter.min = 0;
			counter.hour++;
		}
	}
	
}
	
u8 TIM_Set(TIM_TypeDef* TIMx, FunctionalState NewState)
{
	if(NewState == ENABLE)	//开定时器，开中断
	{
		TIM_Cmd(TIMx,ENABLE);
		TIM_ITConfig(TIMx,TIM_IT_Update,ENABLE);
	}
	else	//关定时器，停止中断
	{
		TIM_Cmd(TIMx,DISABLE);
		TIM_ITConfig(TIMx,TIM_IT_Update,DISABLE);
	}
	return 0;
}







