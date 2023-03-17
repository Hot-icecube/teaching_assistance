#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "lcd.h"
#include "w25qxx_spi_flash.h"
//#include "fontupd.h"
//#include "text.h"
//#include "exfuns.h"
#include "malloc.h"
#include "usart3.h"
#include "hc05.h"
#include "string.h"
#include "GBK_LibDrive.h"
#include "tim.h"
#include "key.h"

#define LINE_SIZE		 16


//���յ���Ϣ��Ϊ�����ִ洢 ��ʱ��ʹ���ʾ���ı�
message_obj mesg_RX_BUF[30];
u8 start_show = 0;


//��ʾATK-HC05ģ�������״̬
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:Connect     ");			//���ӳɹ�
	else LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:DisConnect");		 			//δ����				 
}	

void Counter_Show(void)
{
	LCD_ShowxNum(100,42,counter.min,2,16,RED,0);//��ʾ����
	GBK_Show_Str(45,42,200,200,"��ʱ��   :   ",16, RED, WHITE, 1);
	LCD_ShowxNum(135,42,counter.sec,2,16,RED,0);//��ʾ��
}

int main(void)
{
	u8 reclen = 0; //�������ݳ���
	u8 line = 58;	//��ʼ��ʾ������
	u8 receive_num = 0;//���մ������״�/���״�
//	u8 key = 0;
//	u8 key_press = 0;
	u8 i = 0;
	u8 j = 0;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�ж����ȼ�2��2
	uart_init(115200);//����1 ��ʼ��
	delay_init();
	LCD_Init();
	printf("LCD_ID:%X\r\n",lcddev.id);
	W25Qxx_Init();
	printf("FLASH_ID:0X%X\r\n",W25Qxx_ReadID());
	GBK_Lib_Init();//Ӳ��GBK�ֿ��ʼ��
	KEY_Init();
	TIM_Init(9999,7199);//10Khz�ļ���Ƶ��,  ��ʱ1s == 1000 000us 
	
	
	delay_ms(100);
	LCD_ShowString(30,70,200,16,16,RED,(u8*)"The System is Initing!"); 
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
//	exfuns_init();				//
//	printf("�ڴ����뺯������ֵ��%d\r\n",exfuns_init());//0,���سɹ�	
//	printf("ʵʱʱ�ӳ�ʼ��������ֵ��%d\r\n",RTC_Init());	//0-��ʼ���ɹ�

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


	GBK_Show_Str(5, 26, 200,300,(u8*)"��������������",16, RED, WHITE, 0);
	memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0
	
	
    while(1)
	{
		HC05_Sta_Show();//��ʾ����״̬

		if(USART3_RX_STA & 0x8000)	//���յ�һ������
		{	
			reclen = USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			USART3_RX_BUF[reclen] = 0;//��ӽ�����
			USART3_RX_STA = 0;//������ձ�־λ
			
			mesg_Analysis(&mesg_RX_BUF[receive_num], USART3_RX_BUF);
			
			if(receive_num == 0)//��һ�ν���
			{
				receive_num++;
			}
			else//�ٴν���
			{
				receive_num++;
			}
			if(receive_num >= 11)	
			{
				receive_num = 0;
				line = 58;
				LCD_Clear(WHITE);
				GBK_Show_Str(5, 26, 200,300,"������������:",16, RED, WHITE, 0);
				memset(mesg_RX_BUF, 0, sizeof(mesg_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0	
			}
			
			//printf("receive_num: %d,\t total_time:%d,\t text:%s\r\n",\
					receive_num-1, mesg_RX_BUF[receive_num].total_time, mesg_RX_BUF[receive_num].text);
			
			printf("receive: %s,\t strlen�� %d\r\n",USART3_RX_BUF,strlen((const char*)USART3_RX_BUF));
			
		}
		
	
		//��ʾ���մ���
		LCD_ShowxNum(115,26,receive_num,2,16,RED,0);
		//memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0
		Counter_Show();
			
		if(start_show == 1)
		{
			i=1;
			while(i <= receive_num)
			{
				Counter_Show();
				if( !Is_TimeOut(mesg_RX_BUF[i-1].total_time) )	//��ʱδ��
				{
//					printf("mesg_RX_BUF[%d].total_time :%d\n",i,mesg_RX_BUF[i].total_time);
					//��ʾ���յ�ʱ�估����
					LCD_ShowxNum(10,line,mesg_RX_BUF[i-1].time_min,2,16,RED,0);//��ʾ����
					GBK_Show_Str(30,line,200,200," : ",16, RED, WHITE, 0);
					LCD_ShowxNum(50,line,mesg_RX_BUF[i-1].time_sec,2,16,RED,0);//��ʾ��
					GBK_Show_Str(70,line, 200,200,(u8*)&mesg_RX_BUF[i-1].text,16,BLUE, WHITE, 0);
				}			
				else //��ʱ��
				{
//					printf("mesg_RX_BUF[%d].total_time :%d\n",i,mesg_RX_BUF[i].total_time);
//					printf(" total_time out !!! nowtime is %d : %d\r\n",counter.min,counter.sec);
					counter.hour = counter.min = counter.sec = 0;
					i++;
					line += LINE_SIZE;//��ʾλ������	
					
				}
				/* ���һ����Ϣ��ʾ��ϣ��Ҽ�ʱ��� */
				if( i==receive_num && Is_TimeOut(mesg_RX_BUF[i-1].total_time) )
				{
					//i=0;
					line = 58;
					TIM_Set(TIM4,DISABLE);
					counter.hour = counter.min = counter.sec = 0;
					printf("Ending show\n");
					 
					//��������
					for(j=0;j<8;j++)
					{
						LCD_LED=0;
						delay_ms(300);
						LCD_LED=1;
						delay_ms(300);
					}
					break;
				}
				
			}//while(i<=receive_num)
			
			start_show = 0;
		}
		
	

		
	}//while(1)
	
}



