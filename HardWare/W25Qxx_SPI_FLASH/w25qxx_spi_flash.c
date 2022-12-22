#include "w25qxx_spi_flash.h"

//W25Q128 0xEF17
//W25Q16 0xEF14
u16 Flash_ID = 0;	//flashоƬ�ͺ�

//SPI�ṹ���ʼ��
void SPI_Flash_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef	SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE);		//SPI2 ����APB1
	
	//PB12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������ģʽ
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PB13 PB14 PB15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������ģʽ
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |GPIO_Pin_15);//�øߵ�ƽ,����
	
	//ģʽ3
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//���䷽�򣺵�/˫��   ˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					//����ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				//����֡����
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;					//ʱ�Ӽ���,����ʱ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;				//ʱ����λ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;					//NSS��������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;			//ʱ�ӷ�Ƶ����
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//У��ֵ
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2,ENABLE);		//ʹ��SPI2
	//SPIx_ReadWriteByte(0xFF);
}

/*
*	���ܣ� SPIx ��д����  1�ֽڣ�
*   
*   ������ u8 TxData -- ��д����ֽ�
*			  
*	����ֵ�����յ�������				
*/
u8 SPIx_ReadWriteByte(u8 TxData)
{
	u8 retry = 0;
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)//���ָ����SPI��־λ��������ͻ����
	{
		retry++;
		if(retry > 200)	return 0;
	}
	SPI_I2S_SendData(SPI2, TxData);		//��������
	retry = 0;
	
	//���ָ����SPI��־λ���������ջ����
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
		retry++;
		if(retry > 200)	return 0;
	}
	return (SPI_I2S_ReceiveData(SPI2));		//���ؽ�������

}

/*
*	���ܣ� ��ȡFLASH_ID
*   
*   ������
*	����ֵ��temp ����FLASH ID
*			 
*/
u16 W25Qxx_ReadID(void)
{
	u16 temp = 0;//
	W25Qxx_CS = 0;					//ʹ���ⲿFLASH
	SPIx_ReadWriteByte(0X90);		//д���IDָ��
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 
	temp |= SPIx_ReadWriteByte(0XFF)<< 8;		//
	temp |= SPIx_ReadWriteByte(0XFF);
	
	W25Qxx_CS = 1;					//ȡ��ʹ���ⲿFLASH
	return temp;
}
/*
*	���ܣ� ��ʼ��FLASHоƬ
*   
*   ��������
*			 
*	����ֵ����
*/
void W25Qxx_Init(void)
{
	W25Qxx_CS = 1;
	SPI_Flash_Init();
	Flash_ID = W25Qxx_ReadID();
}

/*
*	���ܣ���FLASH״̬�Ĵ���
*   
*   ��������
*			 
*	����ֵ��u8 byte -- ��ȡ���ļĴ���״̬
*/
u8 W25Qxx_Read_SR(void)
{
	u8 byte;
	W25Qxx_CS = 0;                            //ʹ������   
	SPIx_ReadWriteByte(W25X_ReadStatusReg);		//���Ͷ�ȡ�Ĵ�������
	byte = SPIx_ReadWriteByte(0xFF);         	// ��ȡ״̬ 
	W25Qxx_CS = 1;  
	return	byte;
}
/*
*	���ܣ� д״̬�Ĵ���
*  			ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
*   ������u8 byte -- д����ֽ�
*			 
*	����ֵ����
*/
void W25Qxx_Write_SR(u8 byte)
{
	W25Qxx_CS = 0;                            //ʹ������   
	SPIx_ReadWriteByte(W25X_WriteStatusReg);//����дȡ״̬�Ĵ�������    
	SPIx_ReadWriteByte(byte);               	//д��һ���ֽ�  
	W25Qxx_CS = 1;         					//ȡ��ʹ������
}

/*
*	���ܣ� FLASHдʹ��
*   		��WEL��λ
*   ��������
*			 
*	����ֵ����
*/
void W25Qxx_Write_Enable(void)
{
	W25Qxx_CS=0;                          	//ʹ������   
    SPIx_ReadWriteByte(W25X_WriteEnable); 	//����дʹ��  
	W25Qxx_CS=1;       
}

/*
*	���ܣ� FLASHд��ֹ
*   
*   ��������
*			 
*	����ֵ����
*/
void W25Qxx_Write_Disable(void)
{
	W25Qxx_CS=0;                          	//ʹ������   
    SPIx_ReadWriteByte(W25X_WriteDisable); 	//����дʹ��  
	W25Qxx_CS=1;       
}

//�ȴ�����
void W25Qxx_Wait_Busy(void)   
{   
	while((W25Qxx_Read_SR() & 0x01) == 0x01);  		// �ȴ�BUSYλ���
} 
/*
*	���ܣ� ��������,������ȫΪ1
*  
*   ������u32 SecAddr -- ���������ĵ�ַ
*			 
*	����ֵ����
*/	
void W25Qxx_Erase_Sector(u32 SecAddr)
{
	W25Qxx_Write_Enable();						//дʹ��
	SecAddr *= 4096;
    W25Qxx_Write_Enable();                  	//SET WEL 	 
    W25Qxx_Wait_Busy();   						//
	
  	W25Qxx_CS=0;                            	//ʹ������   
    SPIx_ReadWriteByte(W25X_SectorErase);      	//������������ָ�� 
    SPIx_ReadWriteByte((u8)((SecAddr) >> 16));  	//����24bit��ַ    
    SPIx_ReadWriteByte((u8)((SecAddr) >> 8));   
    SPIx_ReadWriteByte((u8)SecAddr);  
	W25Qxx_CS = 1;                            	//ȡ��Ƭѡ     	
	
    W25Qxx_Wait_Busy();   						//�ȴ��������
}
//ȫ������
void W25Qxx_Erase_Chip(void)   
{                                   
    W25Qxx_Write_Enable();                 	 	//SET WEL 
    W25Qxx_Wait_Busy();   
  	W25Qxx_CS = 0;                            	//ʹ������   
    SPIx_ReadWriteByte(W25X_ChipErase);        	//����Ƭ��������  
	W25Qxx_CS = 1;                            	//ȡ��Ƭѡ     	      
	W25Qxx_Wait_Busy();   				   		//�ȴ�оƬ��������
}   
/*
*	���ܣ� �������ݶ�ȡ
*  
*   ������u8 *pBuffer -- ��ȡ������
*			u32 ReadAddr -- ��ȡ��ַ,24bit
*			u16 NumByteToRead -- ��ȡ���ֽ��� <65535 		 
*	����ֵ����
*/	
void W25Qxx_Read_Data(u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	W25Qxx_CS = 0;  
	//W25Qxx_Write_Enable();
	SPIx_ReadWriteByte(W25X_ReadData);			//���Ͷ�ȡ����
	SPIx_ReadWriteByte((u8)(ReadAddr >> 16));     //����24λ��ַ����8
	SPIx_ReadWriteByte((u8)(ReadAddr >> 8));		//����24λ����8
	SPIx_ReadWriteByte((u8)(ReadAddr));			//����24λ����8
	for(i = 0; i<NumByteToRead; i++){			//���ݶ�ȡ
		pBuffer[i] = SPIx_ReadWriteByte(0xFF);
	}
	W25Qxx_CS = 1;  
}

/*
*	���ܣ� SPI��һҳ(0~65535)��д������256���ֽڵ�����
*			��ָ����ַ��ʼд�����256�ֽڵ�����
*   ������u8 *pBuffer -- ���ݴ洢
*			u32 ReadAddr -- ��ʼд��ַ,24bit
*			u16 NumByteToRead -- д����ֽ��� <65535 		 
*	����ֵ����
*/	
void W25Qxx_Write_Page(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i = 0;
	W25Qxx_Write_Enable();
    W25Qxx_Write_Enable();                  	//SET WEL 
	W25Qxx_CS=0;                            	//ʹ������   
    SPIx_ReadWriteByte(W25X_PageProgram);      	//����дҳ����   
    SPIx_ReadWriteByte((u8)((WriteAddr)>>16)); 	//����24bit��ַ    
    SPIx_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPIx_ReadWriteByte((u8)WriteAddr);   
    for(i=0; i<NumByteToWrite; i++)				//ѭ��д��
		SPIx_ReadWriteByte(pBuffer[i]);  
	
	W25Qxx_CS=1;                            	//ȡ��Ƭѡ 
	W25Qxx_Wait_Busy();					   		//�ȴ�д�����
}
/*
*	���ܣ� �޼���дSPI FLASH
*		  ����ȷ��д�ĵ�ַ��Χ������Ϊ0XFF������ʼ��״̬��������дʧ��
*		  ���Զ���ҳ����
*   ������u8 *pBuffer -- ���ݴ洢��
*		   u32 ReadAddr -- ��ʼд���ַ,24bit
*		   u16 NumByteToWrite -- Ҫд����ֽ���(���65535) 		 
*	����ֵ����
*/	
void W25Qxx_Write_NoCheck(u8 *pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageRemain;	   
	pageRemain = 256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageRemain)	pageRemain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		W25Qxx_Write_Page(pBuffer, WriteAddr, pageRemain);
		if(NumByteToWrite==pageRemain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer += pageRemain;
			WriteAddr += pageRemain;	

			NumByteToWrite -= pageRemain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)	pageRemain = 256; //һ�ο���д��256���ֽ�
			else pageRemain = NumByteToWrite; 	  //����256���ֽ���
		}
	};	    
} 

/*
*	���ܣ� дSPI FLASH
*		  �ƶ���ַ��ʼд��������
*		  ������������д֮ǰ�Ȳ���
*   ������u8 *pBuffer -- ���ݴ洢��
*		   u32 ReadAddr -- ��ʼд���ַ,24bit
*		   u16 NumByteToWrite -- Ҫд����ֽ���(���65535) 		 
*	����ֵ����
*/	

u8 W25Qxx_BUFFER[4096];		 
void W25Qxx_Write(u8 *pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 *W25Qxx_BUF;	  
   	W25Qxx_BUF = W25Qxx_BUFFER;	     
 	secpos = WriteAddr/4096;//������ַ  
	secoff = WriteAddr%4096;//�������ڵ�ƫ��
	secremain = 4096-secoff;//����ʣ��ռ��С   
 
 	if(NumByteToWrite <= secremain)		secremain = NumByteToWrite;//������4096���ֽ� 
	while(1)
	{
		W25Qxx_Read_Data(W25Qxx_BUF, secpos*4096, 4096);//������������������
		for(i=0; i<secremain; i++)//У������
		{
			if(W25Qxx_BUF[secoff+i] != 0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			W25Qxx_Erase_Sector(secpos);		//�����������
			for(i=0; i<secremain; i++)	   		//����
			{
				W25Qxx_BUF[i+secoff]=pBuffer[i];	  
			}
			
		W25Qxx_Write_NoCheck(W25Qxx_BUF, secpos*4096, 4096);//д����������  
		}else W25Qxx_Write_NoCheck(pBuffer, WriteAddr, secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite == secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff = 0;//ƫ��λ��Ϊ0 	 

		   	pBuffer += secremain;  				//ָ��ƫ��
			WriteAddr += secremain;				//д��ַƫ��	   
		   	NumByteToWrite -= secremain;			//�ֽ����ݼ�
			if(NumByteToWrite > 4096)	secremain=4096;//��һ����������д����
			else secremain = NumByteToWrite;		//��һ����������д����
		}
	};		
}

//����FLASH
void W25Qxx_PowerDown(void)
{
	W25Qxx_CS = 0;
	SPIx_ReadWriteByte(W25X_PowerDown);
	W25Qxx_CS = 1;

}
//����FLASH
void W25Qxx_WakeUp(void)
{
	W25Qxx_CS = 0;
	SPIx_ReadWriteByte(W25X_ReleasePowerDown);
	W25Qxx_CS = 1;
}

