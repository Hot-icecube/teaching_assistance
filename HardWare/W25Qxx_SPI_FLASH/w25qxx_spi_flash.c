#include "w25qxx_spi_flash.h"

//W25Q128 0xEF17
//W25Q16 0xEF14
u16 Flash_ID = 0;	//flash芯片型号

//SPI结构体初始化
void SPI_Flash_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef	SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE);		//SPI2 挂载APB1
	
	//PB12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PB13 PB14 PB15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |GPIO_Pin_15);//置高电平,上拉
	
	//模式3
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//传输方向：单/双，   双线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					//主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				//数据帧长度
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;					//时钟极性,空闲时
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;				//时钟相位
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;					//NSS引脚设置
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;			//时钟分频因子
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//校验值
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2,ENABLE);		//使能SPI2
	//SPIx_ReadWriteByte(0xFF);
}

/*
*	功能： SPIx 读写命令  1字节，
*   
*   参数表： u8 TxData -- 待写入的字节
*			  
*	返回值：接收到的数据				
*/
u8 SPIx_ReadWriteByte(u8 TxData)
{
	u8 retry = 0;
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)//检查指定的SPI标志位设置与否发送缓存空
	{
		retry++;
		if(retry > 200)	return 0;
	}
	SPI_I2S_SendData(SPI2, TxData);		//发送数据
	retry = 0;
	
	//检查指定的SPI标志位设置与否接收缓存空
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
		retry++;
		if(retry > 200)	return 0;
	}
	return (SPI_I2S_ReceiveData(SPI2));		//返回接收数据

}

/*
*	功能： 读取FLASH_ID
*   
*   参数表：
*	返回值：temp 返回FLASH ID
*			 
*/
u16 W25Qxx_ReadID(void)
{
	u16 temp = 0;//
	W25Qxx_CS = 0;					//使能外部FLASH
	SPIx_ReadWriteByte(0X90);		//写入读ID指令
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 	    
	SPIx_ReadWriteByte(0x00); 
	temp |= SPIx_ReadWriteByte(0XFF)<< 8;		//
	temp |= SPIx_ReadWriteByte(0XFF);
	
	W25Qxx_CS = 1;					//取消使能外部FLASH
	return temp;
}
/*
*	功能： 初始化FLASH芯片
*   
*   参数表：无
*			 
*	返回值：无
*/
void W25Qxx_Init(void)
{
	W25Qxx_CS = 1;
	SPI_Flash_Init();
	Flash_ID = W25Qxx_ReadID();
}

/*
*	功能：读FLASH状态寄存器
*   
*   参数表：无
*			 
*	返回值：u8 byte -- 读取到的寄存器状态
*/
u8 W25Qxx_Read_SR(void)
{
	u8 byte;
	W25Qxx_CS = 0;                            //使能器件   
	SPIx_ReadWriteByte(W25X_ReadStatusReg);		//发送读取寄存器命令
	byte = SPIx_ReadWriteByte(0xFF);         	// 读取状态 
	W25Qxx_CS = 1;  
	return	byte;
}
/*
*	功能： 写状态寄存器
*  			只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
*   参数表：u8 byte -- 写入的字节
*			 
*	返回值：无
*/
void W25Qxx_Write_SR(u8 byte)
{
	W25Qxx_CS = 0;                            //使能器件   
	SPIx_ReadWriteByte(W25X_WriteStatusReg);//发送写取状态寄存器命令    
	SPIx_ReadWriteByte(byte);               	//写入一个字节  
	W25Qxx_CS = 1;         					//取消使能器件
}

/*
*	功能： FLASH写使能
*   		将WEL置位
*   参数表：无
*			 
*	返回值：无
*/
void W25Qxx_Write_Enable(void)
{
	W25Qxx_CS=0;                          	//使能器件   
    SPIx_ReadWriteByte(W25X_WriteEnable); 	//发送写使能  
	W25Qxx_CS=1;       
}

/*
*	功能： FLASH写禁止
*   
*   参数表：无
*			 
*	返回值：无
*/
void W25Qxx_Write_Disable(void)
{
	W25Qxx_CS=0;                          	//使能器件   
    SPIx_ReadWriteByte(W25X_WriteDisable); 	//发送写使能  
	W25Qxx_CS=1;       
}

//等待空闲
void W25Qxx_Wait_Busy(void)   
{   
	while((W25Qxx_Read_SR() & 0x01) == 0x01);  		// 等待BUSY位清空
} 
/*
*	功能： 扇区擦除,擦除后全为1
*  
*   参数表：u32 SecAddr -- 擦除扇区的地址
*			 
*	返回值：无
*/	
void W25Qxx_Erase_Sector(u32 SecAddr)
{
	W25Qxx_Write_Enable();						//写使能
	SecAddr *= 4096;
    W25Qxx_Write_Enable();                  	//SET WEL 	 
    W25Qxx_Wait_Busy();   						//
	
  	W25Qxx_CS=0;                            	//使能器件   
    SPIx_ReadWriteByte(W25X_SectorErase);      	//发送扇区擦除指令 
    SPIx_ReadWriteByte((u8)((SecAddr) >> 16));  	//发送24bit地址    
    SPIx_ReadWriteByte((u8)((SecAddr) >> 8));   
    SPIx_ReadWriteByte((u8)SecAddr);  
	W25Qxx_CS = 1;                            	//取消片选     	
	
    W25Qxx_Wait_Busy();   						//等待擦除完成
}
//全部擦除
void W25Qxx_Erase_Chip(void)   
{                                   
    W25Qxx_Write_Enable();                 	 	//SET WEL 
    W25Qxx_Wait_Busy();   
  	W25Qxx_CS = 0;                            	//使能器件   
    SPIx_ReadWriteByte(W25X_ChipErase);        	//发送片擦除命令  
	W25Qxx_CS = 1;                            	//取消片选     	      
	W25Qxx_Wait_Busy();   				   		//等待芯片擦除结束
}   
/*
*	功能： 扇区数据读取
*  
*   参数表：u8 *pBuffer -- 读取的数据
*			u32 ReadAddr -- 读取地址,24bit
*			u16 NumByteToRead -- 读取的字节数 <65535 		 
*	返回值：无
*/	
void W25Qxx_Read_Data(u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u16 i;
	W25Qxx_CS = 0;  
	//W25Qxx_Write_Enable();
	SPIx_ReadWriteByte(W25X_ReadData);			//发送读取命令
	SPIx_ReadWriteByte((u8)(ReadAddr >> 16));     //发送24位地址，高8
	SPIx_ReadWriteByte((u8)(ReadAddr >> 8));		//发送24位，中8
	SPIx_ReadWriteByte((u8)(ReadAddr));			//发送24位，低8
	for(i = 0; i<NumByteToRead; i++){			//数据读取
		pBuffer[i] = SPIx_ReadWriteByte(0xFF);
	}
	W25Qxx_CS = 1;  
}

/*
*	功能： SPI在一页(0~65535)内写入少于256个字节的数据
*			在指定地址开始写入最大256字节的数据
*   参数表：u8 *pBuffer -- 数据存储
*			u32 ReadAddr -- 开始写地址,24bit
*			u16 NumByteToRead -- 写入的字节数 <65535 		 
*	返回值：无
*/	
void W25Qxx_Write_Page(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i = 0;
	W25Qxx_Write_Enable();
    W25Qxx_Write_Enable();                  	//SET WEL 
	W25Qxx_CS=0;                            	//使能器件   
    SPIx_ReadWriteByte(W25X_PageProgram);      	//发送写页命令   
    SPIx_ReadWriteByte((u8)((WriteAddr)>>16)); 	//发送24bit地址    
    SPIx_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPIx_ReadWriteByte((u8)WriteAddr);   
    for(i=0; i<NumByteToWrite; i++)				//循环写数
		SPIx_ReadWriteByte(pBuffer[i]);  
	
	W25Qxx_CS=1;                            	//取消片选 
	W25Qxx_Wait_Busy();					   		//等待写入结束
}
/*
*	功能： 无检验写SPI FLASH
*		  首先确保写的地址范围内数据为0XFF（即初始化状态），否则写失败
*		  可自动换页功能
*   参数表：u8 *pBuffer -- 数据存储区
*		   u32 ReadAddr -- 开始写入地址,24bit
*		   u16 NumByteToWrite -- 要写入的字节数(最大65535) 		 
*	返回值：无
*/	
void W25Qxx_Write_NoCheck(u8 *pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageRemain;	   
	pageRemain = 256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageRemain)	pageRemain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25Qxx_Write_Page(pBuffer, WriteAddr, pageRemain);
		if(NumByteToWrite==pageRemain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer += pageRemain;
			WriteAddr += pageRemain;	

			NumByteToWrite -= pageRemain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)	pageRemain = 256; //一次可以写入256个字节
			else pageRemain = NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 

/*
*	功能： 写SPI FLASH
*		  制定地址开始写定长数据
*		  带擦除操作，写之前先擦除
*   参数表：u8 *pBuffer -- 数据存储区
*		   u32 ReadAddr -- 开始写入地址,24bit
*		   u16 NumByteToWrite -- 要写入的字节数(最大65535) 		 
*	返回值：无
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
 	secpos = WriteAddr/4096;//扇区地址  
	secoff = WriteAddr%4096;//在扇区内的偏移
	secremain = 4096-secoff;//扇区剩余空间大小   
 
 	if(NumByteToWrite <= secremain)		secremain = NumByteToWrite;//不大于4096个字节 
	while(1)
	{
		W25Qxx_Read_Data(W25Qxx_BUF, secpos*4096, 4096);//读出整个扇区的内容
		for(i=0; i<secremain; i++)//校验数据
		{
			if(W25Qxx_BUF[secoff+i] != 0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25Qxx_Erase_Sector(secpos);		//擦除这个扇区
			for(i=0; i<secremain; i++)	   		//复制
			{
				W25Qxx_BUF[i+secoff]=pBuffer[i];	  
			}
			
		W25Qxx_Write_NoCheck(W25Qxx_BUF, secpos*4096, 4096);//写入整个扇区  
		}else W25Qxx_Write_NoCheck(pBuffer, WriteAddr, secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite == secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff = 0;//偏移位置为0 	 

		   	pBuffer += secremain;  				//指针偏移
			WriteAddr += secremain;				//写地址偏移	   
		   	NumByteToWrite -= secremain;			//字节数递减
			if(NumByteToWrite > 4096)	secremain=4096;//下一个扇区还是写不完
			else secremain = NumByteToWrite;		//下一个扇区可以写完了
		}
	};		
}

//休眠FLASH
void W25Qxx_PowerDown(void)
{
	W25Qxx_CS = 0;
	SPIx_ReadWriteByte(W25X_PowerDown);
	W25Qxx_CS = 1;

}
//唤醒FLASH
void W25Qxx_WakeUp(void)
{
	W25Qxx_CS = 0;
	SPIx_ReadWriteByte(W25X_ReleasePowerDown);
	W25Qxx_CS = 1;
}

