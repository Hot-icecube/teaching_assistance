#include "key.h"
#include "tim.h"
#include "usart.h"


extern key_table line_local;

//ɨ�谴��
void KEY_GPIO_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOE,ENABLE);
	GPIO_InitTypeDef GPIO_InitStucture;
	
	//KEY0--PE4 ,KEY1--PE3
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_3| GPIO_Pin_4;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOE,&GPIO_InitStucture);
	
	//WK_UP--PAO
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPD;//��������
	GPIO_Init(GPIOA,&GPIO_InitStucture);
	
}


//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0���ް�������
//1��KEY0����
//2��KEY1����
//3��WK_UP���� 
//ע��˺�������Ӧ���ȼ�,KEY0 > KEY1 > WK_UP
u8 KEY_Scan(u8 mode)
{
	u8 key_up = 1;//�������ɿ���־
	if(mode)	key_up=1;  //֧������		  
	if(key_up && (KEY0==0 || KEY1==0 || WK_UP==1))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)		return KEY0_PRES;
		else if(KEY1==0)	return KEY1_PRES;
		else if(WK_UP==1)	return WKUP_PRES;
	}else if(KEY0==1 && KEY1==1 && WK_UP==0)	key_up=1; 	    
 	return 0;// �ް������� 
}



//�ⲿ�ж�
void KEY_EXTI_Init()
{
	EXTI_DeInit();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//��������IO��ʱ��
	
	KEY_GPIO_Init();  
	
	/* ����IO�����ж��ߵ�ӳ�� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);	
	
	/*��ʼ�������жϣ����ô������� */
	//PE4
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;  //PE4/PE3ѡ����4
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//ѡ��
	EXTI_Init(&EXTI_InitStructure);
	
	//PE3
	EXTI_InitStructure.EXTI_Line = EXTI_Line3 ;  //PE3ѡ����3
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½���
	EXTI_Init(&EXTI_InitStructure);
	
	
	//PA0
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 ;  //PA0ѡ����0
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//ѡ��������
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_ClearITPendingBit(EXTI_Line4| EXTI_Line3| EXTI_Line0);
	
	//�����жϺ����飨NVIC������ʹ��
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //��Ϊѡ���ж����ȼ�Ϊ2������ֻ��ѡ0-3��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
}


extern u8 start_show;
u8 key_press=0;

//KEY0 �ж��ӳ���
void EXTI4_IRQHandler()
{
	delay_ms(10);
	if( EXTI_GetITStatus(EXTI_Line4) != RESET)//
    {	
		if(KEY0 == 0)	//KEY0������
		{
			if(key_press == 0)	//��һ�ΰ��£���ʼ��ʱ
			{
				
				key_press = 1;//����״̬��ת
				TIM_Set(TIM4,ENABLE);//����ʱ��
				start_show = 1;
//				printf("key0 is press\n");
				
			}
			else	//��ͣ��ʱ
			{
				key_press = 0;//����״̬��ת
				TIM_Set(TIM4,DISABLE);//�رն�ʱ��
				start_show = 0;
//				printf("key0 is press again\n");
			}	

		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
  
	
}

//KEY1 �ж��ӳ���
void EXTI3_IRQHandler()
{
	
	delay_ms(10);
	if( EXTI_GetITStatus(EXTI_Line3) != RESET)//
    {	
		if(KEY1 == 0)	//KEY1������
		{
	
			line_local.current++;
			
//			printf("key1 press\n");
				
		}
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
  
}

//WK_UP �ж��ӳ���
void EXTI0_IRQHandler()
{
	
	delay_ms(10);
	if( EXTI_GetITStatus(EXTI_Line0) != RESET)//
    {	
		if(WK_UP == 1)	//KEY1������
		{
//			printf("WK-UP press\n");
		
		}
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
  
}


void KEY_Init()
{
	KEY_GPIO_Init();
	
#if KEY_MODE		//ѡ�񰴼�ʶ��ʽ
	KEY_EXTI_Init();
#endif	
	
}


