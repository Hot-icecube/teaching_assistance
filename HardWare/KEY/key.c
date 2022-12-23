#include "key.h"
#include "delay.h"

void KEY_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOE,ENABLE);
	GPIO_InitTypeDef GPIO_InitStucture;
	
	//KEY0--PE4 ,KEY1--PE3
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPU;//下拉输入
	GPIO_Init(GPIOE,&GPIO_InitStucture);
	
	//WK_UP--PAO
	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPD;//下拉输入
	GPIO_Init(GPIOA,&GPIO_InitStucture);
	
	
}


//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，无按键按下
//1，KEY0按下
//2，KEY1按下
//3，WK_UP按下 
//注意此函数有响应优先级,KEY0 > KEY1 > WK_UP
u8 KEY_Scan(u8 mode)
{
	u8 key_up = 1;//按键按松开标志
	if(mode)	key_up=1;  //支持连按		  
	if(key_up && (KEY0==0 || KEY1==0 || WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)		return KEY0_PRES;
		else if(KEY1==0)	return KEY1_PRES;
		else if(WK_UP==1)	return WKUP_PRES;
	}else if(KEY0==1 && KEY1==1 && WK_UP==0)	key_up=1; 	    
 	return 0;// 无按键按下 
}






