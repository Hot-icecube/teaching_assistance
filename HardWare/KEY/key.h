#ifndef _KEY_H
#define _KEY_H

#include "delay.h"
#include "sys.h"
#include "stm32f10x.h"


typedef struct
{
	u8 current;
	u8 back;		//向上翻索引号
	u8 next;		//向下翻索引号
	
} key_table;





#define KEY0  	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0
#define KEY1  	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//读取按键1
#define WK_UP 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 


#define KEY0_PRES 	1	//KEY0按下
#define KEY1_PRES	2	//KEY1按下
#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)

#define KEY_MODE    1  	//1,中断方式; 0-扫描方式


void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数	


#endif 

