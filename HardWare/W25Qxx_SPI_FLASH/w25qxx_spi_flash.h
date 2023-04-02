#ifndef _W25QXX_SPI_FLASH
#define _W25QXX_SPI_FLASH

#include "stm32f10x.h"
#include "sys.h"

//全局变量声明
extern u16 Flash_ID;//
#define FLASH_SIZE 64*1024*1024
#define W25Qxx_CS	PBout(12)			//W25Q片选信号

//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

//函数声明
void SPI_Flash_Init(void);
u8 SPIx_ReadWriteByte(u8 TxData);
void W25Qxx_Init(void);
u16 W25Qxx_ReadID(void);
void W25Qxx_Init(void);

u8 W25Qxx_Read_SR(void);
void W25Qxx_Write_SR(u8 byte);

void W25Qxx_Write_Enable(void);
void W25Qxx_Write_Disable(void);
void W25Qxx_Wait_Busy(void) ;

void W25Qxx_Erase_Sector(u32 SecAddr);
void W25Qxx_Erase_Chip(void);

void W25Qxx_Read_Data(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

void W25Qxx_Write_Page(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void W25Qxx_Write_NoCheck(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite);  
void W25Qxx_Write(u8 *pBuffer, u32 WriteAddr,u16 NumByteToWrite);
void W25Qxx_PowerDown(void);
void W25Qxx_WakeUp(void);



#endif		/* _W25QXX_SPI_FLASH */
