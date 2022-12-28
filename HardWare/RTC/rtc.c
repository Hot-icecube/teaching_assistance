#include "rtc.h"
#include "usart.h"
#include "delay.h"

calendar_ calendar;//

/*
*	���ܣ� RTC�ж�����
*   
*   ������ ��
*			  
*	����ֵ����			
*/
static void RTC_NVIC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*
*	���ܣ� RTC��ʼ������
*   
*   ������ ��
*			  
*	����ֵ��0 - ��ʼ���ɹ�		
*/
u8 RTC_Init(void)
{
	u8 temp = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������
	
	/*RTC �������� */
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x50)//��ָ���ĺ󱸼Ĵ����ж������ݣ������
	{
		BKP_DeInit();
		RCC_LSEConfig(RCC_LSE_ON);
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET && temp<250)
		{
			temp++;
			delay_ms(10);
		}
		if(temp >= 250)	return 1;	//��ʼ��ʱ��ʧ��
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//ѡ���ⲿ��Դ��LSE��ΪRTCʱ��
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();
		
		RTC_ITConfig(RTC_IT_SEC,ENABLE);	//ʹ�����ж�
		RTC_WaitForLastTask();
		
		/* ��������ģʽ*/
		RTC_EnterConfigMode();
		RTC_SetPrescaler(32767);	//����RTCԤ��Ƶֵ
		RTC_WaitForLastTask();
		RTC_SetTime(2022,12,23,18,00,00);//���ó�ʼʱ��
		RTC_ExitConfigMode();
		BKP_WriteBackupRegister(BKP_DR1, 0x5050);//��ָ���󱸼Ĵ�����д������
	}
	
	else //RTC�ѳ�ʼ����������ʱ
	{
		RTC_WaitForSynchro();		//�ȴ����һ�ζ�RTC�Ĵ���д�������
		RTC_ITConfig(RTC_IT_SEC,ENABLE);
		RTC_WaitForLastTask();
	}
	RTC_NVIC_Init();
	RTC_GetTime();	//����ʱ��
	return 0;
	
}


//RTC,�жϷ�����
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	RTC_GetTime();//�����ж�
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);//��������ж�
		RTC_GetTime();
		printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.year,calendar.month,calendar.date,
												calendar.hour,calendar.min,calendar.sec);//�������ʱ��
	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//����жϱ�־λ
	RTC_WaitForLastTask();
}


/*
*	���ܣ� �ж��Ƿ�Ϊ����
*		�·�   1  2  3  4  5  6  7  8  9  10 11 12
*		����   31 29 31 30 31 30 31 31 30 31 30 31
*		������ 31 28 31 30 31 30 31 31 30 31 30 31
*
*   ������ u16 year -- ��
*		  
*	����ֵ��1 -- �� ��0 -- ����
*/
u8 Is_LeapYear(u16 year)
{
	if(year%4 == 0)
	{
		if(year%100 == 0)
		{
			if(year%400 == 0)	return 1;
			else return 0;
		}
		else return 1;
	}
	else return 0;
}

/*
*	���ܣ� ����ʱ��ת��Ϊ����
*		
*   ������ u16 year,[1970-2099]
*		  
*	����ֵ��0 -- �ɹ���1 -- ʧ��
*/
u8 const table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
const u8 table_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//ƽ����·����ڱ�


u8 RTC_SetTime(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u16 secCount;
	if(year<1970 || year>2099)	return 1;//�������󣬷���
	for(t = 1970; t<year; t++)
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;		//����������366*24*3600
		else	secCount += SEC_YEAR;
	}
	month -= 1;
	for(t=0 ;t<month; t++)	   //��ǰ���·ݵ����������
	{
		secCount += (u32)table_month[t]*SEC_DAY;		//�·����������
		if(Is_LeapYear(year) && t==1) 	secCount += SEC_DAY;		//����2�·�����һ���������	   
	}
	secCount += (u32)(day-1)*SEC_DAY;	//�ۼ�ǰ�������
	secCount += (u32)hour*SEC_HOUR;		//Сʱ������
    secCount += (u32)min*60;	 		//����������
	secCount += sec;		//�������Ӽ���ȥ 	
	
	//����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������  
	RTC_SetCounter(secCount);
	
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	return 0;
}



/*
*	���ܣ� ��ȡ����
*   
*   ������ u16 year -- ��
*			u8 month -- ��
*			u8 day -- ��
*			  
*	����ֵ��������1-7
*/

u8 RTC_GetWeek(u16 year, u8 month, u8 day)
{
	u16 temp;
	u8 yearH,yearL;
	
	yearH = year/100;
	yearL = year%100;
	
	//21���ͣ�year+100
	if(yearH>19)	yearL += 100;
	temp = yearL+yearL/4;
	temp = temp%7; 
	temp = temp+day+table_week[month-1];
	if (yearL%4==0 && month<3)	temp--;
	return(temp%7);
}



/*
*	���ܣ� ��ȡʱ�䣬��ʱ��ֵ�洢��ʱ��ṹ����
*		
*   ������ ��
*		  
*	����ֵ��0 -- �ɹ���1 -- ʧ��
*/
u8 RTC_GetTime(void)
{
	u16 dayCount = 0;
	u32 timeCount = 0;
	u32 temp = 0;
	u32 temp1 = 0;
	
	timeCount = RTC_GetCounter();//��ȡ������
	temp = timeCount/SEC_DAY;//������������/ÿ��������
	
	if(dayCount != temp) //
	{
		dayCount = temp;
		temp1 = 1970;	//��ʼ����
		//�������
		while(temp >=365)	//����һ��ʱ���ж��Ƿ�����
		{
			if(Is_LeapYear(temp1))//������
			{
				if(temp >= 366)		temp -=366;
				else 	{ temp1++; break;	}//ͳ�����
			}
			else temp -=365;//��ȥƽ�������
			temp1++;
		}
		calendar.year = temp1;//�õ����
		
		
		temp1=0;
		
		while(temp >= 28)//����һ����
		{
			if(Is_LeapYear(calendar.year) && temp1==1)//���������꣬�ҵ���Ϊ2��
			{
				if(temp >= 29)	temp -=29;//��ȥ��������Ϊ2�µ�����
				else break;
			}
			else	//ƽ��
			{
				if(temp >= table_month[temp1])	temp -= table_month[temp1];//���·ݱ����ҵ���Ӧ�·ݲ�����Ӧ����
			}
			temp1++;
		}
		calendar.month = temp1+1;//��
		calendar.date = temp+1;//����
	}
	temp = timeCount%SEC_DAY;//�õ�������
	
	calendar.hour = temp/SEC_HOUR;//ʱ
	calendar.hour = (temp%SEC_HOUR)/SEC_MIN;//��
	calendar.sec = (temp%SEC_HOUR)%SEC_MIN;//��
	
	calendar.week = RTC_GetWeek(calendar.year,calendar.month,calendar.date);
	return 0;
}

/*
*	���ܣ� ��������
*		
*   ������ u16 year,[1970-2099]
*		  
*	����ֵ��0 -- �ɹ���1 -- ʧ��
*/
u8 RTC_SetClock(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u32 secCount = 0;
	if(year<1970 || year>2099)	return 1;	   
	for(t=1970; t<year; t++)	//��������ݵ��������
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;//�����������
		else secCount += SEC_YEAR;			  //ƽ���������
	}
	month -= 1;
	for(t=0;t<month;t++)	   //��ǰ���·ݵ����������
	{
		secCount += table_month[t]*SEC_DAY;//�·����������
		if(Is_LeapYear(year) && t==1)	secCount += SEC_DAY;//����2�·�����һ���������	   
	}
	
	secCount += (u32)(day-1)*SEC_DAY;//��ǰ�����ڵ���������� 
	secCount += (u32)hour*SEC_HOUR;//Сʱ������
    secCount += (u32)min*SEC_MIN;	 //����������
	secCount += sec;//�������Ӽ���ȥ 	
	
	//����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������  
	//���������Ǳ����!
	
	RTC_SetAlarm(secCount);
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	
	return 0;
}
