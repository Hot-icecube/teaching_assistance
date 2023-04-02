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
			


//接收的信息分为两部分存储 ，时间和待显示的文本
message_obj mesg_RX_BUF[MESSAGE_MAX_SIZE];
u8 start_show = 0;
key_table line_local={0,0,0};


//显示ATK-HC05模块的连接状态
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:Connect     ");			//连接成功
	else LCD_ShowString(5,10,120,16,16,BLACK,(u8*)"STA:DisConnect");		 			//未连接				 
}	

void Counter_Show(void)
{
	LCD_ShowxNum(85,42,counter.hour,2,16,RED,0);//显示小时
	GBK_Show_Str(30,42,200,200,"计时器:",16, RED, WHITE, 0);
	GBK_Show_Str(100,42,200,200,"	:	  : 	",16, RED, WHITE, 0);
	LCD_ShowxNum(120,42,counter.min,2,16,RED,0);//显示分钟
	LCD_ShowxNum(150,42,counter.sec,2,16,RED,0);//显示秒
}

int main(void)
{
	u8 reclen = 0; //接收数据长度
	u8 line = 58;	//开始显示的行数
	u8 receive_num = 0;//接收次数，首次/非首次
	u16 time_color = BLACK;
	u16 text_color= BLUE;
	//u16 back_color = WHITE;
	counter_obj temp_counter[MESSAGE_MAX_SIZE];
	u8 temp_buf[1024];		//缓冲区
	u16 len;
	
	u8 i = 0;
	u8 j = 0;
	u8 x = 0;
	u8 time_out_flag = 0;
	u8 last_current=0;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//中断优先级2：2
	uart_init(115200);//串口1 初始化
	delay_init();
	LCD_Init();
	printf("LCD_ID:%X\r\n",lcddev.id);
	W25Qxx_Init();
	printf("FLASH_ID:0X%X\r\n",W25Qxx_ReadID());
	GBK_Lib_Init();//硬件GBK字库初始化
	KEY_Init();
	TIM_Init(9999,7199);//10Khz的计数频率,  延时1s == 1000 000us 
	
	
	delay_ms(100);
	LCD_ShowString(30,70,200,16,16,RED,(u8*)"The System is Initing!"); 
//	my_mem_init(SRAMIN);		//初始化内部内存池
//	exfuns_init();				//
//	printf("内存申请函数返回值：%d\r\n",exfuns_init());//0,返回成功	
//	printf("实时时钟初始化，返回值：%d\r\n",RTC_Init());	//0-初始化成功

//	f_mount(fs[1], "1", 1);//挂载外部flash
	
//	while(font_init())//字库是否挂载成功
//	{
//		printf("字库挂载失败\r\n");
//		LCD_ShowString(30, 100, 200, 200, 24, (u8*)"Font mounting error");//挂载失败
//	}
	
	
	while(HC05_Init()) 		//初始化ATK-HC05模块  
	{
		LCD_ShowString(30,90,200,16,16,RED,(u8*)"ATK-HC05 Error!"); 
		delay_ms(200);
		LCD_ShowString(30,90,200,16,16,RED,(u8*)"Please Check!!!"); 
		delay_ms(200);
	}
	
	LCD_Clear(WHITE);

	
	
	memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//清空数组内元素，使其为0
		
		
    while(1)
	{
		HC05_Sta_Show();//显示连接状态

		if(USART3_RX_STA & 0x8000)	//接收到一次数据
		{	
			reclen = USART3_RX_STA&0X7FFF;	//得到数据长度
			USART3_RX_BUF[reclen] = 0;//添加结束符
			USART3_RX_STA = 0;//清除接收标志位
			
			mesg_Analysis(&mesg_RX_BUF[receive_num], USART3_RX_BUF);
			receive_num++;
			
			//printf("receive_num: %d,\t total_time:%d,\t text:%s\r\n",\
					receive_num-1, mesg_RX_BUF[receive_num].total_time, mesg_RX_BUF[receive_num].text);
			
			printf("receive: %s,\t strlen： %d\r\n",USART3_RX_BUF,strlen((const char*)USART3_RX_BUF));
			
		}
		
		//显示接收次数
		GBK_Show_Str(5, 26, 200,300,(u8*)"接收数据条数：",16, RED, WHITE, 0);
		LCD_ShowxNum(115,26,receive_num,2,16,RED,0);
		//memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));//清空数组内元素，使其为0
		Counter_Show();
			
		if(start_show == 1)//开始计时
		{
			
			i=1;
			while(i <= receive_num)
			{                            
				Counter_Show();//计时器显示
				LCD_ShowxNum(5,line,mesg_RX_BUF[i-1].time_min,2,24,time_color,0);//显示分钟
				GBK_Show_Str(28,line,200,200,(u8*)" : ",16, time_color, WHITE, 0);
				LCD_ShowxNum(50,line,mesg_RX_BUF[i-1].time_sec,2,24,time_color,0);//显示秒
				GBK_Show_Str(85,line, 200,200,(u8*)&mesg_RX_BUF[i-1].text,24,text_color, WHITE, 0);//显示文本
				
				if(i == line_local.current) //指向当前行，特殊显示
				{									
					time_color = RED;
					text_color = RED;
					line += LINE_SIZE;
					
					//记录切换时的时间
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
				//时间是否到达设定值
				time_out_flag = Is_TimeOut(mesg_RX_BUF[x].time_min, mesg_RX_BUF[x].time_sec);
				if(time_out_flag == 1)
				{
					x++;
					//闪屏
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
				if(line_local.current%MAX_LINE == 0 && line_local.current!=0)	//指向页尾,开始下一页显示
				{	
					if (line_local.current != last_current)//如果相等，说明当前已经执行过 LCD_Clear，不需要再次执行
					{
						last_current = line_local.current;
						LCD_Clear(WHITE);					
						line=58;
					}
				}
				
				
			}//while(i<=receive_num)
			
			if(start_show!=1 || i>=receive_num)//发送计时数据
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
				start_show = 0;//标志清0
				//printf("%s\n",temp_buf);
				
			}		
		}
		
	
	}//while(1)
}






