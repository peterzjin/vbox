/*******************************************************************************
 * @file	miniSTM32_diskio.c
 * @author	Brian
 * @version	V0.2.0
 * @date	17-August-2011
 * @brief	This file provides HAL (hardware abstraction layer) for FatFs.
 * @note	For compatibility, sector(block) size is fixed to 512, and only 
 *			supported drive	number is 0.
 * @verbatim
 *               
 *			Programming Model 
 *          ======================================= 
 *			// Mount FAT file system
 *			f_mount(0, &fatfs);
 *
 *			// File read operation
 *			rc = f_open(&fil, "filname.ext", FA_READ);
 *			while(1) {
 *				rc = f_read(&fil, buffer, sizeof(buffer), &br);
 *				if( rc || !br ) break;
 *			}
 *			rc = f_close(&fil);
 *
 *			// File write operation
 *			rc = f_open(&fil, "filename.ext", FA_WRITE | FA_CREATE_ALWAYS);
 *			rc = f_write(&fil, "Test data" 700, &br);
 *			rc = f_close(&fil);
 *               
 *			// Directory read
 *			rc = f_opendir(&dir, "");
 *			rc = f_readdir(&dir, &fno);
 *              
 *  @endverbatim                
 */ 

#include "mmc_sd.h"					/* SD card subsystem */
#include "diskio.h"					/* FatFs support */

#define FATTIME_YEAR				(2011)
#define FATTIME_MONTH				(1)
#define FATTIME_DATE				(1)
#define FAT_SECTORSIZE				512


/**
 * @brief	Get disk status.
 * @param	drv	: drive number (should be 0)
 * @retval	DSTATUS
 *			This value can be
 *				STA_NODISK	: invalid drive number
 *				STA_NOINIT	: initialization required
 *				STA_OK		: disk is ready
 */
DSTATUS disk_status ( BYTE drv )
{
	/* only one drive is supported */
	if( drv != 0 )  return STA_NODISK;

	/* check if SD card is ready to use */
	if( SDC_GetState() != SD_CARD_TRANSFER )
		return STA_NOINIT;
	else 
		return STA_OK;

}

/**
 * @brief	Initialize the disk.
 * @param	drv	: drive number (should be 0)
 * @retval	DSTATUS
 *			This value can be
 *				STA_NODISK	: invalid drive number
 *				STA_NOINIT	: initialization required
 *				STA_OK		: requested operation succeeded
 */
DSTATUS disk_initialize ( BYTE drv )
{
	/* only one drive is supported */
	if(drv != 0) return STA_NODISK;

	if(SDC_Init() == SD_OK)
	{
		/* It is important to set the block size to FAT_SECTORSIZE here,
		   otherwise subsequent disk operations would freeze */
		if(SDC_SetBlockSize(FAT_SECTORSIZE) != SD_OK)
			return STA_NOINIT;
		else
			return STA_OK;
	}
	else
		return STA_NOINIT;
}

/**
 * @brief	Read sectors from the disk.
 * @param	drv		: drive number (should be 0)
 *			buff	: buffer for the data to be read
 *			sector	: starting sector number
 *			count	: number of sector to be written
 * @retval	DRESULT
 *			This value can be
 *				RES_OK		: requested write operation succeeded
 *				RES_ERROR	: R/W error(check the hardware)
 *				RES_WRPRT	: SD card is write procted(not supported)
 *				RES_NOTRDY	: disk not ready(initialization required)
 *				RES_PARERR	: invalid parameter(check the parameter)
 */

DRESULT disk_read (BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
	SDCardState s;

	if( drv != 0 ) return RES_PARERR;
	if( !count ) return RES_PARERR;

	s = SDC_GetState();

	if( (s == SD_CARD_READY) || (s == SD_CARD_IDENTIFICATION) )
		return RES_NOTRDY;
	else if( (s == SD_CARD_ERROR) || (s == SD_CARD_DISCONNECTED) )
		return RES_ERROR;
	else if( s != SD_CARD_TRANSFER )
		while( SDC_GetState() != SD_CARD_TRANSFER );

	sector *= FAT_SECTORSIZE; /* Convert LBA to byte address */

	if( count == 1 )
	{
		SDC_ReadBlock( buff, sector, FAT_SECTORSIZE );
	}
	else
	{
		SDC_ReadMultiBlocks( buff, sector, FAT_SECTORSIZE, count );
	}
	SDC_WaitReadOperation();

	while(SDC_GetStatus() != SD_TRANSFER_OK);

	return RES_OK;

}


/**
 * @brief	Write sectors to the disk.
 * @param	drv		: drive number (should be 0)
 *			buff	: buffer contains the data to be written
 *			sector	: starting sector number
 *			count	: number of sector to be written
 * @retval	DRESULT
 *			This value can be
 *				RES_OK		: requested write operation succeeded
 *				RES_ERROR	: R/W error(check the hardware)
 *				RES_WRPRT	: SD card is write procted(not supported)
 *				RES_NOTRDY	: disk not ready(initialization required)
 *				RES_PARERR	: invalid parameter(check the parameter)
 */

DRESULT disk_write (BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{

	SDCardState s;

	if( drv != 0 ) return RES_PARERR;
	if( !count ) return RES_PARERR;

	s = SDC_GetState();

	if( (s == SD_CARD_READY) || (s == SD_CARD_IDENTIFICATION) )
		return RES_NOTRDY;
	else if( (s == SD_CARD_ERROR) || (s == SD_CARD_DISCONNECTED) )
		return RES_ERROR;
	else if( s != SD_CARD_TRANSFER )
		while( SDC_GetState() != SD_CARD_TRANSFER );

	sector *= FAT_SECTORSIZE; /* Convert LBA to byte address */

	if( count == 1 )
	{
		SDC_WriteBlock((uint8_t*)buff, sector, FAT_SECTORSIZE);
	}
	else
	{
		SDC_WriteMultiBlocks((uint8_t*)buff, sector, FAT_SECTORSIZE, count);
	}
	SDC_WaitWriteOperation();

	while(SDC_GetStatus() != SD_TRANSFER_OK);

	return RES_OK;

}


/**
 * @brief	Perform disk io control.
 * @param	drv		: drive number (should be 0)
 *			ctrl	: control code. 
 *				Supported codes are CTRL_SYNC, CTRL_EARSE_SECTOR
 *				GET_SECTOR_SIZE, GET_SECTOR_COUNT, and GET_BLOCK_SIZE
 *			buff	: contains information required (both way)
 * @retval	DRESULT
 *			This value can be
 *				RES_OK		: requested operation succeeded
 *				RES_ERROR	: R/W error(check the hardware)
 *				RES_WRPRT	: SD card is write procted(not supported)
 *				RES_NOTRDY	: disk not ready(initialization required)
 *				RES_PARERR	: invalid parameter(check the parameter)
 */

DRESULT disk_ioctl ( BYTE drv, BYTE ctrl, void *buff )
{

	if( drv != 0 ) return RES_PARERR;

	if ( disk_status(drv) & STA_NOINIT) return RES_NOTRDY;

	if( ctrl == CTRL_SYNC ) {
		/* do something here */
		return RES_OK;
	}
	else if( ctrl == GET_SECTOR_SIZE ) {
		*(DWORD*)buff = FAT_SECTORSIZE;
		return RES_OK;
	}
#if _USE_MKFS && !_FS_READONLY
	else if( ctrl == GET_SECTOR_COUNT ) {
		*(DWORD*)buff = SDCardInfo.CardCapacity/FAT_SECTORSIZE;
		return RES_OK;
	}
	else if( ctrl == GET_BLOCK_SIZE ) {
		*(DWORD*)buff = SDCardInfo.CardBlockSize;
		return RES_OK;
	}
#endif
	else if( ctrl == CTRL_ERASE_SECTOR ) {
		if(SDC_Erase( (*((DWORD*)buff)) * FAT_SECTORSIZE, 
			(*((DWORD*)buff+1)) * FAT_SECTORSIZE) == SD_OK)
			return RES_OK;
		else
			return RES_ERROR;
	}

	return RES_ERROR;
}

/**
 * @brief	Provides (fixed) time stamp for FAT operation.
 * @param	None
 * @retval	DWORD: fixed time stamp coded in a double word.
 */

DWORD get_fattime (void)
{
	return	((DWORD)(FATTIME_YEAR - 1980) << 25)	/* Fixed */
			| ((DWORD)FATTIME_MONTH << 21)
			| ((DWORD)FATTIME_DATE << 16)
			| ((DWORD)0 << 11)
			| ((DWORD)0 << 5)
			| ((DWORD)0 >> 1);
}

/* END OF FILE */
