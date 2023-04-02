#ifndef __HC05_H
#define __HC05_H	 
#include "sys.h" 
#include "stm32f10x.h"  
   
#define HC05_KEY  	PAout(4) 		//À¶ÑÀ¿ØÖÆKEYÐÅºÅ
#define HC05_LED  	PAin(5)		//À¶ÑÀÁ¬½Ó×´Ì¬ÐÅºÅ
  
u8 HC05_Init(void);
void HC05_CFG_CMD(u8 *str);
u8 HC05_Get_Role(void);
u8 HC05_Set_Cmd(u8* atstr);	     
#endif  


