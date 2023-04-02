#ifndef _KEY_H
#define _KEY_H

#include "delay.h"
#include "sys.h"
#include "stm32f10x.h"


typedef struct
{
	u8 current;
	u8 back;		//���Ϸ�������
	u8 next;		//���·�������
	
} key_table;





#define KEY0  	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//��ȡ����0
#define KEY1  	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//��ȡ����1
#define WK_UP 	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//��ȡ����3(WK_UP) 


#define KEY0_PRES 	1	//KEY0����
#define KEY1_PRES	2	//KEY1����
#define WKUP_PRES   4	//KEY_UP����(��WK_UP/KEY_UP)

#define KEY_MODE    1  	//1,�жϷ�ʽ; 0-ɨ�跽ʽ


void KEY_Init(void);//IO��ʼ��
u8 KEY_Scan(u8 mode);  	//����ɨ�躯��	


#endif 

