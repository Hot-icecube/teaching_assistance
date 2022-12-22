#include "rtc.h"
#include "usart.h"

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
	u8 temp  = 0;
	//RTC��Դ�ͱ���ʱ��ʹ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);		//ʹ��BKP,PWR
	PWR_BackupAccessCmd(ENABLE);		//PWR�󱸼Ĵ�������

	if(BKP_ReadBackupRegister(BKP_DR1) != 0X50)			//��������ʱ�ӣ����������Լ�ʱ����ֵ��Ϊ0X5050
	{
		BKP_DeInit();
		//RTCʱ��Դѡ��
		RCC_LSEConfig(RCC_LSE_ON);		//�����ⲿ��������
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp<250)
		{
			temp++;
		}	
		if(temp >250) 	return 1;		//��ʼ��ʱ�� ʧ��
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//ʹ���ⲿʱ��
		RCC_RTCCLKCmd(ENABLE);		//ʹ��RTCʱ��
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();		//�ȴ��Ĵ���ͬ�����
		
		//RTC����
		RTC_ITConfig(RTC_IT_SEC, ENABLE);	//
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();		//�ȴ��Ĵ���ͬ�����
		RTC_EnterConfigMode();		//����RTC����ģʽ
		
		// RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
		RTC_SetPrescaler(32767);	//RTCԤ��Ƶֵ��
		RTC_WaitForLastTask();
		RTC_SetTime(2022,8,20,15,18,50);
		RTC_ExitConfigMode();		//�˳�����ģʽ
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//��ָ���ĺ󱸼Ĵ�����д���û���������
	}
	else		//ϵͳ�ѳ�ʼ�� ������ʱ
	{
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_SEC,ENABLE);
		RTC_WaitForLastTask();	
	}
	RTC_NVIC_Init();
	RTC_GetTime();
	return 0;
}


//RTC,�жϷ�����
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	RTC_GetTime();//�����ж�
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);
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
*	���ܣ� ��ȡ����
*   
*   ������ u16 year -- ��
*			u8 month -- ��
*			u8 day -- ��
*			  
*	����ֵ��������1-7
*/
u8 const table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
const u8 table_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//ƽ����·����ڱ�

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
*	���ܣ� ����ʱ��ת��Ϊ����
*		
*   ������ u16 year,[1970-2099]
*		  
*	����ֵ��0 -- �ɹ���1 -- ʧ��
*/
u8 RTC_SetTime(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u16 secCount;
	if(year<1970 || year>2099)	return 1;
	for(t = 1970; t<year; t++)
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;		//����366*24*3600
		else	secCount += SEC_YEAR;
	}
	month -= 1;
	for(t=0 ;t<month; t++)	   //��ǰ���·ݵ����������
	{
		secCount+=(u32)table_month[t]*SEC_DAY;		//�·����������
		if(Is_LeapYear(year) && t==1) secCount += SEC_DAY;		//����2�·�����һ���������	   
	}
	secCount += (u32)(day-1)*SEC_DAY;	//�ۼ�ǰ�������
	secCount += (u32)hour*SEC_HOUR;		//Сʱ������
    secCount += (u32)min*60;	 		//����������
	secCount += sec;		//�������Ӽ���ȥ 	
	
	//����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������  
	//���������Ǳ����!
	
	RTC_SetAlarm(secCount);
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	return 0;
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
	static u16 dayCount = 0;
	u32 timeCount = 0;
	u32 tempDay = 0;
	u16 tempYear = 0;
	
	timeCount = RTC_GetCounter();		//��ȡ����
	tempDay = timeCount/SEC_DAY;		//��ȡ����
	if(dayCount != tempDay )	//����һ��
	{
		dayCount = tempDay;
		tempYear = 1970;
		while(tempDay >= 365)
		{
			if(Is_LeapYear(tempYear))	//����
			{
				if(tempDay >=366 )	tempDay -= 366;	//��������
				else
				{
					tempYear++;
					break;
				}
			}
			tempDay -=365;		//��ȡƽ������
			tempYear++;
		}
		calendar.year = tempYear;//�õ����
		
		tempYear = 0;
		while(tempDay >= 28)
		{
			if(Is_LeapYear(calendar.year) && tempYear ==1)	//����/2��
			{
				if(tempDay >=29 )	tempDay -= 29;	//��������
				else	break;	//
			}
			else		//ƽ��
			{
				if(tempDay >= table_month[tempYear] )	tempDay -= table_month[tempYear];//ƽ��
				else break;
			}
			tempYear++;
		}
		calendar.month = tempYear+1;//�õ��·�
		calendar.date = tempDay+1;//��
	}
	tempDay = timeCount%SEC_DAY;		
	calendar.hour = tempDay/SEC_HOUR;		//Сʱ
	calendar.min = (tempDay%SEC_HOUR)/SEC_MIN;		//��
	calendar.sec = (tempDay%SEC_HOUR)%SEC_MIN;		//��
	calendar.week = RTC_GetWeek(calendar.year,calendar.month,calendar.date);//��ȡ����   
	return 0;
}

/*
*	���ܣ� ��������
*		
*   ������ u16 year,[1970-2099],u8 month,u8 day,u8 hour, u8 min,u8 sec
*		  
*	����ֵ��0 -- �ɹ���1 -- ʧ��
*/
u8 RTC_SetClock(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u32 secCount=0;
	if(year<1970 || year>2099)return 1;	   
	for(t = 1970; t<year; t++)	//��������ݵ��������
	{
		if(Is_LeapYear(t))secCount += SEC_LEAPYEAR;//�����������
		else secCount += SEC_YEAR;			  //ƽ���������
	}
	month-=1;
	for(t=0;t<month;t++)	   //��ǰ���·ݵ����������
	{
		secCount+=(u32)table_month[t]*86400;//�·����������
		if(Is_LeapYear(year) && t==1)secCount+=86400;//����2�·�����һ���������	   
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
