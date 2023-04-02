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

		
#define LINE_SIZE		 	24
#define FONT_SIZE			24
#define MAX_LINE			7
			


//���յ���Ϣ��Ϊ�����ִ洢 ��ʱ��ʹ���ʾ���ı�
message_obj mesg_RX_BUF[MESSAGE_MAX_SIZE];
u8 start_show = 0;
key_table line_local={0,0,0};


//��ʾATK-HC05ģ�������״̬
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:Connect     ");			//���ӳɹ�
	else LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:DisConnect");		 			//δ����				 
}	

void Counter_Show(void)
{
	LCD_ShowxNum(85,42,counter.hour,2,16,RED,0);//��ʾСʱ
	GBK_Show_Str(30,42,200,200,"��ʱ��:",16, RED, WHITE, 0);
	GBK_Show_Str(100,42,200,200,"	:	  : 	",16, RED, WHITE, 0);
	LCD_ShowxNum(120,42,counter.min,2,16,RED,0);//��ʾ����
	LCD_ShowxNum(150,42,counter.sec,2,16,RED,0);//��ʾ��
}

int main(void)
{
	u8 reclen = 0; //�������ݳ���
	u8 line = 58;	//��ʼ��ʾ������
	u8 receive_num = 0;//���մ������״�/���״�
	u16 time_color = BLACK;
	u16 text_color= BLUE;
	//u16 back_color = WHITE;
	counter_obj temp_counter[MESSAGE_MAX_SIZE];
	u8 temp_buf[1024];		//������
	u16 len;
	
	u8 i = 0;
	u8 j = 0;
	u8 x = 0;
	u8 time_out_flag = 0;
	u8 last_current=0;
	
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
			receive_num++;
			
			//printf("receive_num: %d,\t total_time:%d,\t text:%s\r\n",\
					receive_num-1, mesg_RX_BUF[receive_num].total_time, mesg_RX_BUF[receive_num].text);
			
			printf("receive: %s,\t strlen�� %d\r\n",USART3_RX_BUF,strlen((const char*)USART3_RX_BUF));
			
		}
		
		//��ʾ���մ���
		GBK_Show_Str(5, 26, 200,300,(u8*)"��������������",16, RED, WHITE, 0);
		LCD_ShowxNum(115,26,receive_num,2,16,RED,0);
		//memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//���������Ԫ�أ�ʹ��Ϊ0
		Counter_Show();
			
		if(start_show == 1)//��ʼ��ʱ
		{
			
			i=1;
			while(i <= receive_num)
			{                            
				Counter_Show();//��ʱ����ʾ
				LCD_ShowxNum(5,line,mesg_RX_BUF[i-1].time_min,2,24,time_color,0);//��ʾ����
				GBK_Show_Str(28,line,200,200,(u8*)" : ",16, time_color, WHITE, 0);
				LCD_ShowxNum(50,line,mesg_RX_BUF[i-1].time_sec,2,24,time_color,0);//��ʾ��
				GBK_Show_Str(85,line, 200,200,(u8*)&mesg_RX_BUF[i-1].text,24,text_color, WHITE, 0);//��ʾ�ı�
				
				if(i == line_local.current) //ָ��ǰ�У�������ʾ
				{									
					time_color = RED;
					text_color = RED;
					line += LINE_SIZE;
					
					//��¼�л�ʱ��ʱ��
					temp_counter[i-1].hour = counter.hour;
					temp_counter[i-1].min = counter.min;
					temp_counter[i-1].sec = counter.sec;
					
					//printf("hour:%d\t min:%d\t sec:%d\t\n",\
							(int)temp_counter[i-1].hour,(int)temp_counter[i-1].min,(int)temp_counter[i-1].sec);
					i++;	
				}
				else
				{
					time_color = BLACK;
					text_color= BLUE;
					
								
				}
				//ʱ���Ƿ񵽴��趨ֵ
				time_out_flag = Is_TimeOut(mesg_RX_BUF[x].time_min, mesg_RX_BUF[x].time_sec);
				if(time_out_flag == 1)
				{
					x++;
					//����
					for(j=0;j<3;j++)
					{
						LCD_LED=0;
						delay_ms(200);
						LCD_LED=1;
						delay_ms(200);
					}
					if(x+1 >= receive_num)	
					{
						time_out_flag = 2;
						break;
					}
				}	
				if(line_local.current%MAX_LINE == 0 && line_local.current!=0)	//ָ��ҳβ,��ʼ��һҳ��ʾ
				{	
					if (line_local.current != last_current)//�����ȣ�˵����ǰ�Ѿ�ִ�й� LCD_Clear������Ҫ�ٴ�ִ��
					{
						last_current = line_local.current;
						LCD_Clear(WHITE);					
						line=58;
					}
				}
				
				
			}//while(i<=receive_num)
			
			if(start_show!=1 || i>=receive_num)//���ͼ�ʱ����
			{
				//len=sizeof(temp_counter)/sizeof(temp_counter[0]);
				for(i=1; i<=receive_num; i++)
				{
					sprintf((char *)temp_buf,"hour:%d,min:%d,sec:%d\n",\
							temp_counter[i-1].hour,temp_counter[i-1].min,temp_counter[i-1].sec);
					u3_printf("%s\n",temp_buf);
					printf("%s\n",temp_buf);
					delay_us(500);
				}
				start_show = 0;//��־��0
				//printf("%s\n",temp_buf);
				
			}		
		}
		
	
	}//while(1)
}






