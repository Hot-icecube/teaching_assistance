#include "rtc.h"
#include "usart.h"
#include "delay.h"

calendar_ calendar;//

/*
*	功能： RTC中断设置
*   
*   参数表： 无
*			  
*	返回值：无			
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
*	功能： RTC初始化设置
*   
*   参数表： 无
*			  
*	返回值：0 - 初始化成功		
*/
u8 RTC_Init(void)
{
	u8 temp = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问
	
	/*RTC 初次设置 */
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x50)//从指定的后备寄存器中读出数据，不相符
	{
		BKP_DeInit();
		RCC_LSEConfig(RCC_LSE_ON);
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET && temp<250)
		{
			temp++;
			delay_ms(10);
		}
		if(temp >= 250)	return 1;	//初始化时钟失败
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//选择外部震荡源（LSE）为RTC时钟
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();
		
		RTC_ITConfig(RTC_IT_SEC,ENABLE);	//使能秒中断
		RTC_WaitForLastTask();
		
		/* 进入设置模式*/
		RTC_EnterConfigMode();
		RTC_SetPrescaler(32767);	//设置RTC预分频值
		RTC_WaitForLastTask();
		RTC_SetTime(2022,12,23,18,00,00);//设置初始时间
		RTC_ExitConfigMode();
		BKP_WriteBackupRegister(BKP_DR1, 0x5050);//向指定后备寄存器区写入数据
	}
	
	else //RTC已初始化，继续计时
	{
		RTC_WaitForSynchro();		//等待最近一次对RTC寄存器写操作完成
		RTC_ITConfig(RTC_IT_SEC,ENABLE);
		RTC_WaitForLastTask();
	}
	RTC_NVIC_Init();
	RTC_GetTime();	//更新时间
	return 0;
	
}


//RTC,中断服务函数
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	RTC_GetTime();//秒钟中断
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);//清除闹钟中断
		RTC_GetTime();
		printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.year,calendar.month,calendar.date,
												calendar.hour,calendar.min,calendar.sec);//输出闹铃时间
	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//清除中断标志位
	RTC_WaitForLastTask();
}


/*
*	功能： 判断是否为闰年
*		月份   1  2  3  4  5  6  7  8  9  10 11 12
*		闰年   31 29 31 30 31 30 31 31 30 31 30 31
*		非闰年 31 28 31 30 31 30 31 31 30 31 30 31
*
*   参数表： u16 year -- 年
*		  
*	返回值：1 -- 是 ，0 -- 不是
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
*	功能： 输入时间转化为秒钟
*		
*   参数表： u16 year,[1970-2099]
*		  
*	返回值：0 -- 成功，1 -- 失败
*/
u8 const table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
const u8 table_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//平年的月份日期表


u8 RTC_SetTime(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u16 secCount;
	if(year<1970 || year>2099)	return 1;//参数错误，返回
	for(t = 1970; t<year; t++)
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;		//闰年秒钟数366*24*3600
		else	secCount += SEC_YEAR;
	}
	month -= 1;
	for(t=0 ;t<month; t++)	   //把前面月份的秒钟数相加
	{
		secCount += (u32)table_month[t]*SEC_DAY;		//月份秒钟数相加
		if(Is_LeapYear(year) && t==1) 	secCount += SEC_DAY;		//闰年2月份增加一天的秒钟数	   
	}
	secCount += (u32)(day-1)*SEC_DAY;	//累加前面的秒数
	secCount += (u32)hour*SEC_HOUR;		//小时秒钟数
    secCount += (u32)min*60;	 		//分钟秒钟数
	secCount += sec;		//最后的秒钟加上去 	
	
	//设置时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
	RTC_SetCounter(secCount);
	
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	return 0;
}



/*
*	功能： 获取星期
*   
*   参数表： u16 year -- 年
*			u8 month -- 月
*			u8 day -- 日
*			  
*	返回值：星期数1-7
*/

u8 RTC_GetWeek(u16 year, u8 month, u8 day)
{
	u16 temp;
	u8 yearH,yearL;
	
	yearH = year/100;
	yearL = year%100;
	
	//21世纪，year+100
	if(yearH>19)	yearL += 100;
	temp = yearL+yearL/4;
	temp = temp%7; 
	temp = temp+day+table_week[month-1];
	if (yearL%4==0 && month<3)	temp--;
	return(temp%7);
}



/*
*	功能： 获取时间，讲时间值存储至时间结构体中
*		
*   参数表： 无
*		  
*	返回值：0 -- 成功，1 -- 失败
*/
u8 RTC_GetTime(void)
{
	u16 dayCount = 0;
	u32 timeCount = 0;
	u32 temp = 0;
	u32 temp1 = 0;
	
	timeCount = RTC_GetCounter();//获取总秒数
	temp = timeCount/SEC_DAY;//天数（总秒数/每天秒数）
	
	if(dayCount != temp) //
	{
		dayCount = temp;
		temp1 = 1970;	//初始年数
		//计算年份
		while(temp >=365)	//超过一年时，判断是否闰年
		{
			if(Is_LeapYear(temp1))//是闰年
			{
				if(temp >= 366)		temp -=366;
				else 	{ temp1++; break;	}//统计年份
			}
			else temp -=365;//减去平年的天数
			temp1++;
		}
		calendar.year = temp1;//得到年份
		
		
		temp1=0;
		
		while(temp >= 28)//超过一个月
		{
			if(Is_LeapYear(calendar.year) && temp1==1)//当年是闰年，且当月为2月
			{
				if(temp >= 29)	temp -=29;//减去是闰年且为2月的天数
				else break;
			}
			else	//平年
			{
				if(temp >= table_month[temp1])	temp -= table_month[temp1];//从月份表中找到对应月份并减对应天数
			}
			temp1++;
		}
		calendar.month = temp1+1;//月
		calendar.date = temp+1;//日期
	}
	temp = timeCount%SEC_DAY;//得到秒钟数
	
	calendar.hour = temp/SEC_HOUR;//时
	calendar.hour = (temp%SEC_HOUR)/SEC_MIN;//分
	calendar.sec = (temp%SEC_HOUR)%SEC_MIN;//秒
	
	calendar.week = RTC_GetWeek(calendar.year,calendar.month,calendar.date);
	return 0;
}

/*
*	功能： 设置闹钟
*		
*   参数表： u16 year,[1970-2099]
*		  
*	返回值：0 -- 成功，1 -- 失败
*/
u8 RTC_SetClock(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u32 secCount = 0;
	if(year<1970 || year>2099)	return 1;	   
	for(t=1970; t<year; t++)	//把所有年份的秒钟相加
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;//闰年的秒钟数
		else secCount += SEC_YEAR;			  //平年的秒钟数
	}
	month -= 1;
	for(t=0;t<month;t++)	   //把前面月份的秒钟数相加
	{
		secCount += table_month[t]*SEC_DAY;//月份秒钟数相加
		if(Is_LeapYear(year) && t==1)	secCount += SEC_DAY;//闰年2月份增加一天的秒钟数	   
	}
	
	secCount += (u32)(day-1)*SEC_DAY;//把前面日期的秒钟数相加 
	secCount += (u32)hour*SEC_HOUR;//小时秒钟数
    secCount += (u32)min*SEC_MIN;	 //分钟秒钟数
	secCount += sec;//最后的秒钟加上去 	
	
	//设置时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
	//上面三步是必须的!
	
	RTC_SetAlarm(secCount);
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	
	return 0;
}
