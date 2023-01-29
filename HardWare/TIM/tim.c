#include "tim.h"
#include "usart.h"

counter_obj counter;	//������

//set_value:�趨��ʱ�䣬
//����ֵ��1�� ����Ԥ��ֵ��0-δ����ʱֵ
u8 Is_TimeOut(u16 set_time)
{
	u8 now_time;
	now_time = counter.hour*38400 + counter.min*60 + counter.sec;
	if(now_time	>= set_time)	//��ʱ�ﵽԤ��ֵ
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
	
	 //��ʱ��TIM4��ʼ��
    TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ    
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
    TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE ); 
	
	//�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);  
	
	TIM_Cmd(TIM4,DISABLE);
}


//��ʱ��4�жϷ������
void TIM4_IRQHandler()   //TIM3�ж�
{   
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
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
	if(NewState == ENABLE)	//����ʱ�������ж�
	{
		TIM_Cmd(TIMx,ENABLE);
		TIM_ITConfig(TIMx,TIM_IT_Update,ENABLE);
	}
	else	//�ض�ʱ����ֹͣ�ж�
	{
		TIM_Cmd(TIMx,DISABLE);
		TIM_ITConfig(TIMx,TIM_IT_Update,DISABLE);
	}
	return 0;
}







