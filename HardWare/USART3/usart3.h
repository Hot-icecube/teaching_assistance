#ifndef _USART3_H
#define _USART3_H

#include "stm32f10x.h"

#define USART3_MAX_RECV_LEN		1024					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		1024					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;	1,接收.


//接收的消息分为，时间和文本
typedef struct{
	u16 time;
	u8 text[1024];
}message_obj;


extern  message_obj receive_BUF[10];


extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态
 
void USART3_Init(u32 bound);				//串口初始化 
void u3_printf(char* fmt,...);

void TIM7_Int_Init(u16 arr,u16 psc);

u8 mesg_Analysis(message_obj *receive_mesg,u8 *buf);

#endif  


