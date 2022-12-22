#include "rtc.h"
#include "usart.h"

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
	u8 temp  = 0;
	//RTC电源和备份时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);		//使能BKP,PWR
	PWR_BackupAccessCmd(ENABLE);		//PWR后备寄存器访问

	if(BKP_ReadBackupRegister(BKP_DR1) != 0X50)			//初次设置时钟，若需掉电后仍计时讲数值改为0X5050
	{
		BKP_DeInit();
		//RTC时钟源选择
		RCC_LSEConfig(RCC_LSE_ON);		//开启外部低速震荡器
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp<250)
		{
			temp++;
		}	
		if(temp >250) 	return 1;		//初始化时钟 失败
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//使能外部时钟
		RCC_RTCCLKCmd(ENABLE);		//使能RTC时钟
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();		//等待寄存器同步完成
		
		//RTC配置
		RTC_ITConfig(RTC_IT_SEC, ENABLE);	//
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();		//等待寄存器同步完成
		RTC_EnterConfigMode();		//进入RTC配置模式
		
		// RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
		RTC_SetPrescaler(32767);	//RTC预分频值，
		RTC_WaitForLastTask();
		RTC_SetTime(2022,8,20,15,18,50);
		RTC_ExitConfigMode();		//退出配置模式
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);	//向指定的后备寄存器中写入用户程序数据
	}
	else		//系统已初始化 继续计时
	{
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_SEC,ENABLE);
		RTC_WaitForLastTask();	
	}
	RTC_NVIC_Init();
	RTC_GetTime();
	return 0;
}


//RTC,中断服务函数
void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)	RTC_GetTime();//秒钟中断
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);
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
*	功能： 获取星期
*   
*   参数表： u16 year -- 年
*			u8 month -- 月
*			u8 day -- 日
*			  
*	返回值：星期数1-7
*/
u8 const table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
const u8 table_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//平年的月份日期表

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
*	功能： 输入时间转化为秒钟
*		
*   参数表： u16 year,[1970-2099]
*		  
*	返回值：0 -- 成功，1 -- 失败
*/
u8 RTC_SetTime(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u16 secCount;
	if(year<1970 || year>2099)	return 1;
	for(t = 1970; t<year; t++)
	{
		if(Is_LeapYear(t))	secCount += SEC_LEAPYEAR;		//闰年366*24*3600
		else	secCount += SEC_YEAR;
	}
	month -= 1;
	for(t=0 ;t<month; t++)	   //把前面月份的秒钟数相加
	{
		secCount+=(u32)table_month[t]*SEC_DAY;		//月份秒钟数相加
		if(Is_LeapYear(year) && t==1) secCount += SEC_DAY;		//闰年2月份增加一天的秒钟数	   
	}
	secCount += (u32)(day-1)*SEC_DAY;	//累加前面的秒数
	secCount += (u32)hour*SEC_HOUR;		//小时秒钟数
    secCount += (u32)min*60;	 		//分钟秒钟数
	secCount += sec;		//最后的秒钟加上去 	
	
	//设置时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  
	//上面三步是必须的!
	
	RTC_SetAlarm(secCount);
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	return 0;
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
	static u16 dayCount = 0;
	u32 timeCount = 0;
	u32 tempDay = 0;
	u16 tempYear = 0;
	
	timeCount = RTC_GetCounter();		//获取秒数
	tempDay = timeCount/SEC_DAY;		//获取天数
	if(dayCount != tempDay )	//超过一天
	{
		dayCount = tempDay;
		tempYear = 1970;
		while(tempDay >= 365)
		{
			if(Is_LeapYear(tempYear))	//闰年
			{
				if(tempDay >=366 )	tempDay -= 366;	//闰年秒数
				else
				{
					tempYear++;
					break;
				}
			}
			tempDay -=365;		//获取平年秒数
			tempYear++;
		}
		calendar.year = tempYear;//得到年份
		
		tempYear = 0;
		while(tempDay >= 28)
		{
			if(Is_LeapYear(calendar.year) && tempYear ==1)	//闰年/2月
			{
				if(tempDay >=29 )	tempDay -= 29;	//闰年天数
				else	break;	//
			}
			else		//平年
			{
				if(tempDay >= table_month[tempYear] )	tempDay -= table_month[tempYear];//平年
				else break;
			}
			tempYear++;
		}
		calendar.month = tempYear+1;//得到月份
		calendar.date = tempDay+1;//天
	}
	tempDay = timeCount%SEC_DAY;		
	calendar.hour = tempDay/SEC_HOUR;		//小时
	calendar.min = (tempDay%SEC_HOUR)/SEC_MIN;		//分
	calendar.sec = (tempDay%SEC_HOUR)%SEC_MIN;		//秒
	calendar.week = RTC_GetWeek(calendar.year,calendar.month,calendar.date);//获取星期   
	return 0;
}

/*
*	功能： 设置闹钟
*		
*   参数表： u16 year,[1970-2099],u8 month,u8 day,u8 hour, u8 min,u8 sec
*		  
*	返回值：0 -- 成功，1 -- 失败
*/
u8 RTC_SetClock(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec)
{
	u16 t;
	u32 secCount=0;
	if(year<1970 || year>2099)return 1;	   
	for(t = 1970; t<year; t++)	//把所有年份的秒钟相加
	{
		if(Is_LeapYear(t))secCount += SEC_LEAPYEAR;//闰年的秒钟数
		else secCount += SEC_YEAR;			  //平年的秒钟数
	}
	month-=1;
	for(t=0;t<month;t++)	   //把前面月份的秒钟数相加
	{
		secCount+=(u32)table_month[t]*86400;//月份秒钟数相加
		if(Is_LeapYear(year) && t==1)secCount+=86400;//闰年2月份增加一天的秒钟数	   
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
