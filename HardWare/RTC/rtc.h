#ifndef	_RTC_H
#define _RTC_H

#include "stm32f10x.h"

typedef struct{
	vu8 sec;	//秒
	vu8 min;	//分
	vu8 hour;	//时
	vu8 date;	//日
	vu8 week;	//星期
	vu8 month;	//月
	vu16 year;	//年
}calendar_;

extern calendar_ calendar;

//秒宏定义
#define SEC_LEAPYEAR		(SEC_DAY*366)//31,622,400
#define SEC_YEAR			(SEC_DAY*365)//31,536,000
#define SEC_DAY				(SEC_HOUR*24)//86400
#define SEC_HOUR			(SEC_MIN*60)//3600
#define SEC_MIN					60




void RTC_NVIC_Init(void);
u8 RTC_Init(void);
void RTC_IRQHandler(void);
	
u8 Is_LeapYear(u16 year);	//是闰年返回1
u8 RTC_GetWeek(u16 year, u8 month, u8 day);
u8 RTC_SetTime(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec);
u8 RTC_GetTime(void);
u8 RTC_SetClock(u16 year,u8 month,u8 day,u8 hour, u8 min,u8 sec);

#endif /*	_RTC_H	*/
