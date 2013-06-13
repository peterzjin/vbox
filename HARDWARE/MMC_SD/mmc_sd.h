/*******************************************************************************
 * @file	miniSTM32_sdc.h
 * @author	Brian
 * @version	V0.2.0
 * @date	17-August-2011
 * @brief	This file contains public definitions and functions prototypes for
 *			miniSTM32 onboard SD card system. Most of the code came from ST
 *			standard peripheral library V3.5.0 (stm32_eval_sdio_sd.h).
 */ 

#ifndef __MINISTM32_SDC_H
#define __MINISTM32_SDC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"

#define SD_POLLING_MODE

/* default SDIO Data transfer mode */  
#if (!defined(SD_DMA_MODE) && !defined(SD_POLLING_MODE))
	#define SD_DMA_MODE
#endif

/*Supported SD Memory Cards */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1				((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0				((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD					((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD						((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD					((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD				((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD			((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD					((uint32_t)0x00000007)

/**
 * @brief	SD error state
 */
typedef enum
{
  SD_CMD_CRC_FAIL			= (1), /* Command response received with CRC error */
  SD_DATA_CRC_FAIL			= (2), /* Data sent/received with CRC error */
  SD_CMD_RSP_TIMEOUT		= (3), /* Command response timeout */
  SD_DATA_TIMEOUT			= (4), /* Data time out */
  SD_TX_UNDERRUN			= (5), /* Transmit FIFO under-run */
  SD_RX_OVERRUN				= (6), /* Receive FIFO over-run */
  SD_START_BIT_ERR			= (7), /* Start bit not detected in wide bus mode */
  SD_CMD_OUT_OF_RANGE		= (8), /* CMD's argument was out of range.*/
  SD_ADDR_MISALIGNED		= (9), /* Misaligned address */
  SD_BLOCK_LEN_ERR			= (10), /* Transferred block length error */
  SD_ERASE_SEQ_ERR			= (11), /* error in the sequence of erase command */
  SD_BAD_ERASE_PARAM		= (12), /* An Invalid selection for erase groups */
  SD_WRITE_PROT_VIOLATION	= (13), /* Attempt to write to a protect block */
  SD_LOCK_UNLOCK_FAILED		= (14), /* error in unlock/lock command */
  SD_COM_CRC_FAILED			= (15), /* CRC check of the command failed */
  SD_ILLEGAL_CMD			= (16), /* Command is not legal for the card state */
  SD_CARD_ECC_FAILED		= (17), /* ECC was failed to correct the data */
  SD_CC_ERROR				= (18), /* Internal card controller error */
  SD_GENERAL_UNKNOWN_ERROR	= (19), /* General or Unknown error */
  SD_STREAM_READ_UNDERRUN	= (20), /* Error in stream read operation. */
  SD_STREAM_WRITE_OVERRUN	= (21), /* Error in stream program operation */
  SD_CID_CSD_OVERWRITE		= (22), /* CID/CSD overwrite error */
  SD_WP_ERASE_SKIP			= (23), /* only partial address space was erased */
  SD_CARD_ECC_DISABLED		= (24), /* Command has been executed without ECC */
  SD_ERASE_RESET			= (25), /* Out of erase sequence command */
  SD_AKE_SEQ_ERROR			= (26), /* Error in sequence of authentication. */
  SD_INVALID_VOLTRANGE		= (27),
  SD_ADDR_OUT_OF_RANGE		= (28),
  SD_SWITCH_ERROR			= (29),
  SD_SDIO_DISABLED			= (30),
  SD_SDIO_FUNCTION_BUSY		= (31),
  SD_SDIO_FUNCTION_FAILED	= (32),
  SD_SDIO_UNKNOWN_FUNCTION	= (33),
  SD_INTERNAL_ERROR, 
  SD_NOT_CONFIGURED,
  SD_REQUEST_PENDING, 
  SD_REQUEST_NOT_APPLICABLE, 
  SD_INVALID_PARAMETER,  
  SD_UNSUPPORTED_FEATURE,  
  SD_UNSUPPORTED_HW,  
  SD_ERROR,  
  SD_OK = 0 
} SD_Error;

/** 
 * @brief  SDIO Transfer state  
 */   
typedef enum
{
  SD_TRANSFER_OK  = 0,
  SD_TRANSFER_BUSY = 1,
  SD_TRANSFER_ERROR
} SDTransferState;


/** 
 * @brief  SD Card States 
 */   
typedef enum
{
  SD_CARD_READY				= ((uint32_t)0x00000001),
  SD_CARD_IDENTIFICATION	= ((uint32_t)0x00000002),
  SD_CARD_STANDBY			= ((uint32_t)0x00000003),
  SD_CARD_TRANSFER			= ((uint32_t)0x00000004),
  SD_CARD_SENDING			= ((uint32_t)0x00000005),
  SD_CARD_RECEIVING			= ((uint32_t)0x00000006),
  SD_CARD_PROGRAMMING		= ((uint32_t)0x00000007),
  SD_CARD_DISCONNECTED		= ((uint32_t)0x00000008),
  SD_CARD_ERROR				= ((uint32_t)0x000000FF)
} SDCardState;

/** 
 * @brief  Card Specific Data: CSD Register   
 */ 
typedef struct
{
	__IO uint8_t  CSDStruct;			/* CSD structure */
	__IO uint8_t  SysSpecVersion;		/* System specification version */
	__IO uint8_t  Reserved1;			/* Reserved */
	__IO uint8_t  TAAC;					/* Data read access-time 1 */
	__IO uint8_t  NSAC;					/* Data read access-time 2 cycles */
	__IO uint8_t  MaxBusClkFrec;		/* Max. bus clock frequency */
	__IO uint16_t CardComdClasses;		/* Card command classes */
	__IO uint8_t  RdBlockLen;			/* Max. read data block length */
	__IO uint8_t  PartBlockRead;		/* Partial blocks for read allowed */
	__IO uint8_t  WrBlockMisalign;		/* Write block misalignment */
	__IO uint8_t  RdBlockMisalign;		/* Read block misalignment */
	__IO uint8_t  DSRImpl;				/* DSR implemented */
	__IO uint8_t  Reserved2;			/* Reserved */
	__IO uint32_t DeviceSize;			/* Device Size */
	__IO uint8_t  MaxRdCurrentVDDMin;	/* Max. read current @ VDD min */
	__IO uint8_t  MaxRdCurrentVDDMax;	/* Max. read current @ VDD max */
	__IO uint8_t  MaxWrCurrentVDDMin;	/* Max. write current @ VDD min */
	__IO uint8_t  MaxWrCurrentVDDMax;	/* Max. write current @ VDD max */
	__IO uint8_t  DeviceSizeMul;		/* Device size multiplier */
	__IO uint8_t  EraseGrSize;			/* Erase group size */
	__IO uint8_t  EraseGrMul;			/* Erase group size multiplier */
	__IO uint8_t  WrProtectGrSize;		/* Write protect group size */
	__IO uint8_t  WrProtectGrEnable;	/* Write protect group enable */
	__IO uint8_t  ManDeflECC;			/* Manufacturer default ECC */
	__IO uint8_t  WrSpeedFact;			/* Write speed factor */
	__IO uint8_t  MaxWrBlockLen;		/* Max. write data block length */
	__IO uint8_t  WriteBlockPaPartial;	/* Partial blocks for write allowed */
	__IO uint8_t  Reserved3;			/* Reserded */
	__IO uint8_t  ContentProtectAppli;	/* Content protection application */
	__IO uint8_t  FileFormatGrouop;		/* File format group */
	__IO uint8_t  CopyFlag;				/* Copy flag (OTP) */
	__IO uint8_t  PermWrProtect;		/* Permanent write protection */
	__IO uint8_t  TempWrProtect;		/* Temporary write protection */
	__IO uint8_t  FileFormat;			/* File Format */
	__IO uint8_t  ECC;					/* ECC code */
	__IO uint8_t  CSD_CRC;				/* CSD CRC */
	__IO uint8_t  Reserved4;			/* always 1*/
} SD_CSD;

/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct
{
	__IO uint8_t  ManufacturerID;		/* ManufacturerID */
	__IO uint16_t OEM_AppliID;			/* OEM/Application ID */
	__IO uint32_t ProdName1;			/* Product Name part1 */
	__IO uint8_t  ProdName2;			/* Product Name part2*/
	__IO uint8_t  ProdRev;				/* Product Revision */
	__IO uint32_t ProdSN;				/* Product Serial Number */
	__IO uint8_t  Reserved1;			/* Reserved1 */
	__IO uint16_t ManufactDate;			/* Manufacturing Date */
	__IO uint8_t  CID_CRC;				/* CID CRC */
	__IO uint8_t  Reserved2;			/* always 1 */
} SD_CID;

/** 
 * @brief SD Card Status 
 */
typedef struct
{
	__IO uint8_t DAT_BUS_WIDTH;
	__IO uint8_t SECURED_MODE;
	__IO uint16_t SD_CARD_TYPE;
	__IO uint32_t SIZE_OF_PROTECTED_AREA;
	__IO uint8_t SPEED_CLASS;
	__IO uint8_t PERFORMANCE_MOVE;
	__IO uint8_t AU_SIZE;
	__IO uint16_t ERASE_SIZE;
	__IO uint8_t ERASE_TIMEOUT;
	__IO uint8_t ERASE_OFFSET;
} SD_CardStatus;


/** 
 * @brief SD Card information 
 */
typedef struct
{
	SD_CSD SD_csd;
	SD_CID SD_cid;
	uint32_t CardCapacity;  /* Card Capacity */
	uint32_t CardBlockSize; /* Card Block Size */
	uint16_t RCA;
	uint8_t CardType;
} SD_CardInfo;


SD_Error SDC_Init(void);
SD_Error SDC_ReadBlock(uint8_t *Buff, uint32_t Addr, uint16_t BlockSize);
SD_Error SDC_ReadMultiBlocks(uint8_t *Buff, uint32_t Addr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SDC_WriteBlock(uint8_t *Buff, uint32_t Addr, uint16_t BlockSize);
SD_Error SDC_WriteMultiBlocks(uint8_t *Buff, uint32_t Addr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SDC_Erase(uint32_t StartAddr, uint32_t EndAddr);
SD_Error SDC_WaitReadOperation(void);
SD_Error SDC_WaitWriteOperation(void);
SD_Error SDC_ProcessIRQSrc(void);
SD_Error SDC_SetBlockSize(uint32_t BlockSize);
SDCardState SDC_GetState(void);
SDTransferState SDC_GetStatus(void);


#ifdef __cplusplus
}
#endif

#endif /*  __MINISTM32_SDC_H */

/* End of File */
