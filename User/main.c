#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "lcd.h"
#include "w25qxx_spi_flash.h"
//#include "fontupd.h"
#include "text.h"
//#include "exfuns.h"
#include "malloc.h"
//#include "rtc.h"
#include "usart3.h"
#include "hc05.h"
#include "string.h"
#include "GBK_LibDrive.h"
#include "tim.h"
#include "key.h"

#define LINE_SIZE		 16



//��ʾATK-HC05ģ�������״̬
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:Connect     ");			//���ӳɹ�
	else LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:DisConnect");		 			//δ����				 
}	


//����ֵ��1�� ����Ԥ��ֵ������-
u8 Is_TimeOut()
{
	u8 now_time;
	now_time = counter.hour*38400+counter.min*60+counter.sec;
	if(now_time	> counter_value)	//��ʱ�ﵽԤ��ֵ
	{
		now_time = 0;
		
		return 1;
	}
	return 0;
}


int main(void)
{
	u8 reclen = 0; //�������ݳ���
	u8 line = 62;//��ʾ����
	u8 receive_num = 0;//���մ������״�/���״�
	u8 key = 0;
	u8 key_press = 0;
	
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�ж����ȼ�2��2
	uart_init(115200);//����1 ��ʼ��
	delay_init();
	LCD_Init();
	printf("LCD_ID:%X\r\n",lcddev.id);
	W25Qxx_Init();
	printf("FLAHS_ID:0X%X\r\n",W25Qxx_ReadID());
	GBK_Lib_Init();//Ӳ��GBK�ֿ��ʼ��
	KEY_Init();
	TIM_Init(9999,7199);//10Khz�ļ���Ƶ��,  ��ʱ1s == 1000 000us 
	
	
	delay_ms(100);
	LCD_ShowString(30,70,200,16,16,RED,(u8*)"The System is Initing!"); 
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
//	exfuns_init();				//
//	printf("�ڴ����뺯������ֵ��%d\r\n",exfuns_init());//0,���سɹ�	
//	printf("ʵʱʱ�ӳ�ʼ��������ֵ��%d\r\n",RTC_Init());	//0-��ʼ���ɹ�
//	RTC_Init();

//	f_mount(fs[1], "1", 1);//�����ⲿflash
	
//	while(font_init())//�ֿ��Ƿ���سɹ�
//	{
//		printf("�ֿ����ʧ��\r\n");
//		LCD_ShowString(30, 100, 200, 200, 24, (u8*)"Font mounting error");//����ʧ��
//	}
	
	
	while(HC05_Init()) 		//��ʼ��ATK-HC05ģ��  
	{
		LCD_ShowString(30,90,200,16,16,RED,(u8*)"ATK-HC05 Error!"); 
		delay_ms(200);
		LCD_ShowString(30,90,200,16,16,RED,(u8*)"Please Check!!!"); 
		delay_ms(200);
	}
	
	LCD_Clear(WHITE);
	GBK_Show_Str(50,30, 200,300,(u8*)"���ֱ��ݽ�����ϵͳ",16, RED, WHITE, 0);
	GBK_Show_Str(5, 46, 200,300,(u8*)"��������Ϊ��",16, RED, WHITE, 0);
	memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0
	
	
	while(1)
	{
  		key = KEY_Scan(1);
		if(key == KEY0_PRES)
		{
			if(key_press == 0)	//��һ�ΰ��£���ʼ��ʱ
			{
				key_press = 1;//����״̬��ת
				TIM_Set(TIM4,ENABLE);//����ʱ��
			}
			else	//��ͣ��ʱ
			{
				key_press = 0;//����״̬��ת
				TIM_Set(TIM4,DISABLE);//�رն�ʱ��
				
			}
		}
		
		counter_value = 60;
		if(Is_TimeOut())
		{
			TIM_Set(TIM4,DISABLE);
		    printf("time out !!! nowtime is %d\n",counter.sec);
			counter.hour = counter.min = counter.sec = 0;
		
		}			
		
		LCD_ShowxNum(80,220,counter.min,2,16,RED,0);//��ʾ����
		LCD_ShowxNum(100,220,counter.sec,2,16,RED,0);//��ʾ��
		
		
	}	//while(1)
//	while(1)
//	{
//		HC05_Sta_Show();//��ʾ����״̬

//		if(USART3_RX_STA & 0x8000)	//���յ�һ������
//		{	
//			reclen = USART3_RX_STA&0X7FFF;	//�õ����ݳ���
//			USART3_RX_BUF[reclen] = 0;//��ӽ�����
//			USART3_RX_STA = 0;//������ձ�־λ
//			
//			if(receive_num == 0)//��һ�ν���
//			{
//				receive_num++;
//			}
//			else//�ٴν���
//			{
//				line += LINE_SIZE;//��ʾλ������
//				receive_num++;
//			}
//		}	
//		GBK_Show_Str(8,line, 500,500,  USART3_RX_BUF,16, RED, WHITE, 0);
//		
//		if(receive_num >13)	
//		{
//			LCD_Clear(WHITE);
//			GBK_Show_Str(50,30, 200,300,"���ֱ��ݽ�����ϵͳ",16, RED, WHITE, 0);
//			GBK_Show_Str(5, 46, 200,300,"��������Ϊ��",16, RED, WHITE, 0);
//			receive_num = 0;
//			line = 62;
//		}
//		memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0						
//		

//		
//	}//while(1)
	
}



