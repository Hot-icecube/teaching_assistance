#ifndef _USART3_H
#define _USART3_H

#include "stm32f10x.h"

#define USART3_MAX_RECV_LEN		1024					//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		1024					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;	1,����.


extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   						//��������״̬
 
void USART3_Init(u32 bound);				//���ڳ�ʼ�� 
void u3_printf(char* fmt,...);

void TIM7_Int_Init(u16 arr,u16 psc);

#endif  
