#include "key.h"
#include "tim.h"
#include "usart.h"

//扫描按键
void KEY_GPIO_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOE,ENABLE);
	GPIO_InitTypeDef GPIO_InitStucture;
	
	//KEY0--PE4 ,KEY1--PE3
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOE,&GPIO_InitStucture);
	
	//WK_UP--PAO
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPD;//下拉输入
	GPIO_Init(GPIOA,&GPIO_InitStucture);
	
}


//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，无按键按下
//1，KEY0按下
//2，KEY1按下
//3，WK_UP按下 
//注意此函数有响应优先级,KEY0 > KEY1 > WK_UP
u8 KEY_Scan(u8 mode)
{
	u8 key_up = 1;//按键按松开标志
	if(mode)	key_up=1;  //支持连按		  
	if(key_up && (KEY0==0 || KEY1==0 || WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)		return KEY0_PRES;
		else if(KEY1==0)	return KEY1_PRES;
		else if(WK_UP==1)	return WKUP_PRES;
	}else if(KEY0==1 && KEY1==1 && WK_UP==0)	key_up=1; 	    
 	return 0;// 无按键按下 
}



//外部中断
void KEY_EXTI_Init()
{
	EXTI_DeInit();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//开启复用IO口时钟
	
	KEY_GPIO_Init();  
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);	//设置IO口与中断线的映射
	
	//初始化线上中断，设置触发条件
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line4 ;  //PE4/PE3选择线4,3
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//选择中断还是事件触发模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//选择下降沿
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_ClearITPendingBit(EXTI_Line4);
	
	
	//配置中断函分组（NVIC），并使能
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //因为选择中断优先级为2，所以只能选0-3；
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
	
}



extern u8 start_show;
u8 key_press=0;
void EXTI4_IRQHandler()
{
	
	delay_ms(10);
	if( EXTI_GetITStatus(EXTI_Line4) != RESET)//
    {	
		if(KEY0 == 0)	//KEY0被按下
		{
			if(key_press == 0)	//第一次按下，开始计时
			{
				key_press = 1;//按键状态反转
				TIM_Set(TIM4,ENABLE);//开定时器
				start_show = 1;
	
//				printf("key0 is press\n");
			}
			else	//暂停计时
			{
				key_press = 0;//按键状态反转
				TIM_Set(TIM4,DISABLE);//关闭定时器
//				printf("key0 is press again\n");
			}	

		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
  
	
}


void KEY_Init()
{
	KEY_GPIO_Init();
	
#if KEY_MODE
	KEY_EXTI_Init();
#endif	
	
}


