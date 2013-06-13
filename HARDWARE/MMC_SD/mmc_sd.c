/*******************************************************************************
 * @file	miniSTM32_sdc.c
 * @author	Brian
 * @version	V0.2.0
 * @date	17-August-2011
 * @brief	This file provides a set of functions needed to manage the SDIO SD 
 *          Card memory mounted on miniSTM32 board. Most of the code came from
 *			ST standard pepheral library V3.5.0 (stm32_eval_sdio_sd.c). 
 * @note	For compatibility, sector(block) size is fixed to 512, and only 
 *			supported drive	number is 0.
 * @verbatim
 *               
 *			Programming Model 
 *          ======================================= 
 *			// Choose the mode (preferably in the Makefile)
 *			#define SD_DMA_MODE
 *			// or
 *			#define SD_POLLING_MODE
 *
 *			// Initialization
 *			Status = SDC_Init(); 
 *               
 *			// Raw write operation(single block)
 *			Status = SDC_WriteBlock(buffer, address, 512);
 *			Status = SDC_WaitWriteOperation();
 *			while(SDC_GetStatus() != SD_TRANSFER_OK); 
 *             
 *			// Raw write operation(multiple block)
 *			Status = SDC_WriteMultiBlocks(buffer, address, 512, numblocks);
 *			Status = SDC_WaitWriteOperation();
 *			while(SDC_GetStatus() != SD_TRANSFER_OK);     
 *             
 *			// Raw read operation(single block)
 *			Status = SDC_ReadBlock(buffer, address, 512);
 *			Status = SDC_WaitReadOperation();
 *			while(SDC_GetStatus() != SD_TRANSFER_OK);
 *             
 *			// Raw read operation(multiple block)
 *			Status = SDC_ReadMultiBlocks(buffer, address, 512, numblocks);
 *			Status = SDC_WaitReadOperation();
 *			while(SDC_GetStatus() != SD_TRANSFER_OK);            
 *               
 *			// Include the interrupt handling routine in your interrupt code
 *			void SDIO_IRQHandler(void)
 *			{
 *				SDC_ProcessIRQSrc();
 *			}
 *
 *                                     
 *          STM32 SDIO Pin assignment
 *          =========================    
 *          +-----------------------------------------------------------+
 *          |                     Pin assignment                        |
 *          +-----------------------------+---------------+-------------+
 *          |  STM32 SDIO Pins            |     SD        |    Pin      |
 *          +-----------------------------+---------------+-------------+
 *          |      SDIO D2                |   D2          |    1        |
 *          |      SDIO D3                |   D3          |    2        |
 *          |      SDIO CMD               |   CMD         |    3        |
 *          |                             |   VCC         |    4 (3.3 V)|
 *          |      SDIO CLK               |   CLK         |    5        |
 *          |                             |   GND         |    6 (0 V)  |
 *          |      SDIO D0                |   D0          |    7        |
 *          |      SDIO D1                |   D1          |    8        |  
 *          +-----------------------------+---------------+-------------+  
 *              
 *  @endverbatim                
 *             
 */ 

#include "mmc_sd.h"

/* definitions for internal use */
/* SDIO Commands  Index */
#define SD_CMD_GO_IDLE_STATE						((uint8_t)0)
#define SD_CMD_SEND_OP_COND							((uint8_t)1)
#define SD_CMD_ALL_SEND_CID							((uint8_t)2)
#define SD_CMD_SET_REL_ADDR							((uint8_t)3) 
#define SD_CMD_SET_DSR								((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND						((uint8_t)5)
#define SD_CMD_HS_SWITCH							((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD						((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD						((uint8_t)8)
#define SD_CMD_SEND_CSD								((uint8_t)9)
#define SD_CMD_SEND_CID								((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP					((uint8_t)11)
#define SD_CMD_STOP_TRANSMISSION					((uint8_t)12)
#define SD_CMD_SEND_STATUS							((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ						((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE					((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN							((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK					((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK						((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE						((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP					((uint8_t)20)
#define SD_CMD_SET_BLOCK_COUNT						((uint8_t)23)
#define SD_CMD_WRITE_SINGLE_BLOCK					((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK						((uint8_t)25)
#define SD_CMD_PROG_CID								((uint8_t)26)
#define SD_CMD_PROG_CSD								((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT						((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT						((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT						((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START					((uint8_t)32)
#define SD_CMD_SD_ERASE_GRP_END						((uint8_t)33)
#define SD_CMD_ERASE_GRP_START						((uint8_t)35)
#define SD_CMD_ERASE_GRP_END						((uint8_t)36)
#define SD_CMD_ERASE								((uint8_t)38)
#define SD_CMD_FAST_IO								((uint8_t)39)
#define SD_CMD_GO_IRQ_STATE							((uint8_t)40)
#define SD_CMD_LOCK_UNLOCK							((uint8_t)42)
#define SD_CMD_APP_CMD								((uint8_t)55)
#define SD_CMD_GEN_CMD								((uint8_t)56)
#define SD_CMD_NO_CMD								((uint8_t)64)

/* SD Card Specific commands: SDIO_APP_CMD should be sent in prior.  */
#define SD_CMD_APP_SD_SET_BUSWIDTH					((uint8_t)6)
#define SD_CMD_SD_APP_STAUS							((uint8_t)13)
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS			((uint8_t)22)
#define SD_CMD_SD_APP_OP_COND						((uint8_t)41)
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT			((uint8_t)42)
#define SD_CMD_SD_APP_SEND_SCR						((uint8_t)51)
#define SD_CMD_SDIO_RW_DIRECT						((uint8_t)52)
#define SD_CMD_SDIO_RW_EXTENDED						((uint8_t)53)

/* SD Card Specific commands: SDIO_APP_CMD should be sent in prior.  */
#define SD_CMD_SD_APP_GET_MKB						((uint8_t)43)
#define SD_CMD_SD_APP_GET_MID						((uint8_t)44)
#define SD_CMD_SD_APP_SET_CER_RN1					((uint8_t)45)
#define SD_CMD_SD_APP_GET_CER_RN2					((uint8_t)46)
#define SD_CMD_SD_APP_SET_CER_RES2					((uint8_t)47)
#define SD_CMD_SD_APP_GET_CER_RES1					((uint8_t)48)
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK	((uint8_t)18)
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK	((uint8_t)25)
#define SD_CMD_SD_APP_SECURE_ERASE					((uint8_t)38)
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA			((uint8_t)49)
#define SD_CMD_SD_APP_SECURE_WRITE_MKB				((uint8_t)48)

/* Mask for errors Card Status R1 (OCR Register) */
#define SD_OCR_ADDR_OUT_OF_RANGE					((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED						((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR						((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR						((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM						((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION					((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED					((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED						((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD							((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED						((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR								((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR				((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN					((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN					((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE					((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP						((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED					((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET							((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR						((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS							((uint32_t)0xFDFFE008)

/* Masks for R6 Response */
#define SD_R6_GENERAL_UNKNOWN_ERROR					((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD							((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED						((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD						((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY							((uint32_t)0x40000000)
#define SD_STD_CAPACITY								((uint32_t)0x00000000)
#define SD_CHECK_PATTERN							((uint32_t)0x000001AA)

#define	SD_MAX_VOLT_TRIAL							((uint32_t)0x0000FFFF)
#define SD_ALLZERO									((uint32_t)0x00000000)

#define	SD_WIDE_BUS_SUPPORT							((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT						((uint32_t)0x00010000)
#define SD_CARD_LOCKED								((uint32_t)0x02000000)

#define	SD_DATATIMEOUT								((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS									((uint32_t)0x000000FF)
#define SD_8TO15BITS								((uint32_t)0x0000FF00)
#define SD_16TO23BITS								((uint32_t)0x00FF0000)
#define SD_24TO31BITS								((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH							((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO									((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES							((uint32_t)0x00000020)

/* Command Class Supported */
#define SD_CCCC_LOCK_UNLOCK							((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT							((uint32_t)0x00000040)
#define SD_CCCC_ERASE								((uint32_t)0x00000020)

/* SD detection on its memory slot */
#define SD_PRESENT									((uint8_t)0x01)
#define SD_NOT_PRESENT								((uint8_t)0x00)

/* SD Card Specific commands: SDIO_APP_CMD should be sent in prior. */
#define SDIO_SEND_IF_COND							((uint32_t)0x00000008)

/* SDIO Static flags, TimeOut, FIFO Address  */
#define SDIO_STATIC_FLAGS							((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT							((uint32_t)0x00010000)

/* SDIO clock division: SDIO_CK = HCLK/(2 + SDIO_CLK_DIV) */
#define SDIO_INIT_CLK_DIV							((uint8_t)0xB2)	/* 400KHz */
#define SDIO_TRNS_CLK_DIV							((uint8_t)0x01)	/* 24MHz */

/* SDIO IRQ priority parameters */
#define SDIO_IRQPR_PRE								0
#define SDIO_IRQPR_SUB								0
#define SDIO_FIFO_ADDRESS							((uint32_t)0x40018080)

/* SDIO port definitions */
#define SDIO_DAT0_PIN								GPIO_Pin_8
#define SDIO_DAT1_PIN								GPIO_Pin_9
#define SDIO_DAT2_PIN								GPIO_Pin_10
#define SDIO_DAT3_PIN								GPIO_Pin_11
#define SDIO_CLK_PIN								GPIO_Pin_12
#define SDIO_DATA_GPIO_PORT							GPIOC
#define SDIO_DATA_GPIO_CLK							RCC_APB2Periph_GPIOC
#define SDIO_CMD_PIN								GPIO_Pin_2
#define SDIO_CMD_GPIO_PORT							GPIOD
#define SDIO_CMD_GPIO_CLK							RCC_APB2Periph_GPIOD
  
/* variables for internal use */
static uint32_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;
static uint8_t SDSTATUS_Tab[16];
__IO uint32_t StopCondition = 0;
__IO SD_Error TransferError = SD_OK;
__IO uint32_t TransferEnd = 0;

SD_CardInfo SDCardInfo;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;   

/* functions for internal use */
void MCU_SDIOInit(uint8_t ClockDiv, uint32_t BusWide);

void SDC_DeInit(void);
void SDC_DMATxConfig(uint32_t *BufferSRC, uint32_t BufferSize);
void SDC_DMARxConfig(uint32_t *BufferDST, uint32_t BufferSize);
uint32_t SDC_DMAEndOfTransferStatus(void);

SD_Error SDC_PowerOn(void);
SD_Error SDC_PowerOff(void);
SD_Error SDC_InitCards(void);
SD_Error SDC_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SDC_GetCardStatus(SD_CardStatus *cardstatus);
SD_Error SDC_EnableWideBusOperation(uint32_t WideMode);
SD_Error SDC_SelectDeselect(uint32_t addr);
SD_Error SDC_StopTransfer(void);
SD_Error SDC_SendStatus(uint32_t *pcardstatus);
SD_Error SDC_SendSDStatus(uint32_t *psdstatus);
SDTransferState SDC_GetTransferState(void);

static SD_Error SDC_CmdError(void);
static SD_Error SDC_CmdResp1Error(uint8_t cmd);
static SD_Error SDC_CmdResp7Error(void);
static SD_Error SDC_CmdResp3Error(void);
static SD_Error SDC_CmdResp2Error(void);
static SD_Error SDC_CmdResp6Error(uint8_t cmd, uint16_t *prca);

static SD_Error SDC_EnWideBus(FunctionalState NewState);
static SD_Error SDC_IsCardProgramming(uint8_t *pstatus);
static SD_Error SDC_FindSCR(uint16_t rca, uint32_t *pscr);
  
/**
 * @brief	Initialize/Reinitialize SDIO module.
 * @param	ClockDiv:	sdio clock division
 * @param	BusWide:	sdio bus width
 * @retval	None
 */
void MCU_SDIOInit(uint8_t ClockDiv, uint32_t BusWide)
{
	SDIO_InitTypeDef SDIO_InitStructure;

	SDIO_InitStructure.SDIO_ClockDiv = ClockDiv; 
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = BusWide;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init(&SDIO_InitStructure);

}

/**
 * @brief  DeInitializes the SDIO interface.
 * @param  None
 * @retval None
 */
void SDC_DeInit(void)
{ 
	/* Disable SDIO Clock */
	SDIO_ClockCmd(DISABLE);
  
	/* Set Power State to OFF */
	SDIO_SetPowerState(SDIO_PowerState_OFF);

	/* DeInitializes the SDIO peripheral */
	SDIO_DeInit();
  
	/* Disable the SDIO AHB Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, DISABLE);
}

/**
 * @brief	Initializes the SD Card and put it into StandBy State.
 * @param	None
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	SD_Error errorstatus = SD_OK;

	/* GPIOC and GPIOD Periph clock enable */
	RCC_APB2PeriphClockCmd(SDIO_DATA_GPIO_CLK | SDIO_CMD_GPIO_CLK, ENABLE);

	/* configure SDIO D0, D1, D2, D3, CLK pin */
	GPIO_InitStructure.GPIO_Pin = SDIO_DAT0_PIN | SDIO_DAT1_PIN | \
		SDIO_DAT2_PIN | SDIO_DAT3_PIN | SDIO_CLK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SDIO_DATA_GPIO_PORT, &GPIO_InitStructure);

	/* configure SDIO CMD line */
	GPIO_InitStructure.GPIO_Pin = SDIO_CMD_PIN;
	GPIO_Init(SDIO_CMD_GPIO_PORT, &GPIO_InitStructure);

	/* enable the SDIO AHB Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);

#if defined(SD_DMA_MODE)
	/* enable the DMA2 Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
#endif
	
	SDIO_DeInit();
	
	errorstatus = SDC_PowerOn();
	
	if (errorstatus != SD_OK)
	{
		/* CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}
	
	errorstatus = SDC_InitCards();
	
	if (errorstatus != SD_OK)
	{
		/* CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}
	
	/* Configure the SDIO peripheral */
	MCU_SDIOInit(SDIO_TRNS_CLK_DIV, SDIO_BusWide_1b);
	
	if (errorstatus == SD_OK)
	{
		/* Read CSD/CID MSD registers */
		errorstatus = SDC_GetCardInfo(&SDCardInfo);
	}
	
	if (errorstatus == SD_OK)
	{
		/* Select Card */
		errorstatus = SDC_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
	}
	
	/* FIXME: POLLING MODE IS NOT FAST ENOUGH TO USE 4BIT BUS TRANSFER.
	 * You can reduce the clock speed instead of taking 1bit bus transfer 
	 */
#if defined(SD_DMA_MODE)
	if (errorstatus == SD_OK)
	{
		errorstatus = SDC_EnableWideBusOperation(SDIO_BusWide_4b);
	}  
#endif

#if 0
	if( errorstatus == SD_OK )
	{
		NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDIO_IRQPR_PRE;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = SDIO_IRQPR_SUB;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
#endif

	return(errorstatus);
}

/**
 * @brief	Configures the DMA2 Channel4 for SDIO Tx request.
 * @param	BufferSRC: pointer to the source buffer
 * @param	BufferSize: buffer size
 * @retval	None
 */
void SDC_DMATxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Channel4, DISABLE);

	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel4, &DMA_InitStructure);

	/* DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Channel4, ENABLE);  
}

/**
 * @brief	Configures the DMA2 Channel4 for SDIO Rx request.
 * @param	BufferDST: pointer to the destination buffer
 * @param	BufferSize: buffer size
 * @retval	None
 */
void SDC_DMARxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Channel4, DISABLE);

	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)BufferDST;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel4, &DMA_InitStructure);

	/* DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Channel4, ENABLE); 
}

/**
 * @brief	Returns the DMA End Of Transfer Status.
 * @param	None
 * @retval	None
 */
uint32_t SDC_DMAEndOfTransferStatus(void)
{
	return (uint32_t)DMA_GetFlagStatus(DMA2_FLAG_TC4);
}

/**
 * @brief	Gets the cuurent sd card data transfer status.
 * @param	None
 * @retval	SDTransferState: Data Transfer state.
 *   This value can be: 
 *        - SD_TRANSFER_OK: No data transfer is acting
 *        - SD_TRANSFER_BUSY: Data transfer is acting
 */
SDTransferState SDC_GetStatus(void)
{
	SDCardState cardstate =  SD_CARD_TRANSFER;
	
	cardstate = SDC_GetState();
	
	if (cardstate == SD_CARD_TRANSFER)
	{
		return(SD_TRANSFER_OK);
	}
	else if(cardstate == SD_CARD_ERROR)
	{
		return (SD_TRANSFER_ERROR);
	}
	else
	{
		return(SD_TRANSFER_BUSY);
	}
}

/**
 * @brief	Returns the current card's state.
 * @param	None
 * @retval	SDCardState: SD Card Error or SD Card Current State.
 */
SDCardState SDC_GetState(void)
{
	uint32_t resp1 = 0;
  
	if (SDC_SendStatus(&resp1) != SD_OK)
    {
		return SD_CARD_ERROR;
    }
    else
    {
		return (SDCardState)((resp1 >> 9) & 0x0F);
    }
}


/**
 * @brief	Enquires cards about their operating voltage and configures 
 *   clock controls.
 * @param	None
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_PowerOn(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t response = 0, count = 0, validvoltage = 0;
	uint32_t SDType = SD_STD_CAPACITY;
	
	/* Power ON Sequence -----------------------------------------------------*/
	/* Configure the SDIO peripheral */
	/* SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
	/* on STM32F2xx devices, SDIOCLK is fixed to 48MHz */
	/* SDIO_CK for initialization should not exceed 400 KHz */  
	MCU_SDIOInit(SDIO_INIT_CLK_DIV, SDIO_BusWide_1b);
	
	/* Set Power State to ON */
	SDIO_SetPowerState(SDIO_PowerState_ON);
	
	/* Enable SDIO Clock */
	SDIO_ClockCmd(ENABLE);
	
	/* CMD0: GO_IDLE_STATE ---------------------------------------------------*/
	/* No CMD response required */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdError();

	if (errorstatus != SD_OK)
	{
		/* CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}
	
	/* CMD8: SEND_IF_COND ----------------------------------------------------*/
	/* Send CMD8 to verify SD card interface operating condition */
	/* Argument: - [31:12]: Reserved (shall be set to '0')
	- [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
	- [7:0]: Check Pattern (recommended 0xAA) */
	/* CMD Response: R7 */
	SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp7Error();
	
	if (errorstatus == SD_OK)
	{
		CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; /* SD Card 2.0 */
		SDType = SD_HIGH_CAPACITY;
	}
	else
	{
		/* CMD55 */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	}
	/* CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x00;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
	/* If errorstatus is Command TimeOut, it is a MMC card */
	/* If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range 
		mismatch) or SD card 1.x */
	if (errorstatus == SD_OK)
	{
		/* SD CARD */
		/* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			/* SEND CMD55 APP_CMD with RCA as 0 */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
			errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
			errorstatus = SDC_CmdResp3Error();
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
	
			response = SDIO_GetResponse(SDIO_RESP1);
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		}
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return(errorstatus);
		}
	
		if (response &= SD_HIGH_CAPACITY)
		{
			CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		}
	
	}/* else MMC Card */
	
	return(errorstatus);
}

/**
 * @brief	Turns the SDIO output signals off.
 * @param	None
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_PowerOff(void)
{
	SD_Error errorstatus = SD_OK;
	
	/* Set Power State to OFF */
	SDIO_SetPowerState(SDIO_PowerState_OFF);
	
	return(errorstatus);
}

/**
 * @brief	Intialises all cards or single card as the case may be Card(s) come 
 *			into standby state.
 * @param	None
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_InitCards(void)
{
	SD_Error errorstatus = SD_OK;
	uint16_t rca = 0x01;
	
	if (SDIO_GetPowerState() == SDIO_PowerState_OFF)
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return(errorstatus);
	}
	
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
	{
		/* Send CMD2 ALL_SEND_CID */
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
	
		errorstatus = SDC_CmdResp2Error();
	
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	
		CID_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
		CID_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
		CID_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
		CID_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
	}
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) ||  \
		(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||  \
		(SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) ||  \
		(SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/* Send CMD3 SET_REL_ADDR with argument 0 */
		/* SD Card publishes its RCA. */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
	
		errorstatus = SDC_CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);
	
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	}
	
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
	{
		RCA = rca;
	
		/* Send CMD9 SEND_CSD with argument as card's RCA */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
	
		errorstatus = SDC_CmdResp2Error();
	
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	
		CSD_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
		CSD_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
		CSD_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
		CSD_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
	}
	
	errorstatus = SD_OK; /* All cards get intialized */
	
	return(errorstatus);
}

/**
 * @brief	Returns information about specific card.
 * @param	cardinfo: pointer to a SD_CardInfo structure that contains all SD card 
 *			information.
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_GetCardInfo(SD_CardInfo *cardinfo)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;
	
	cardinfo->CardType = (uint8_t)CardType;
	cardinfo->RCA = (uint16_t)RCA;
	
	/* Byte 0 */
	tmp = (uint8_t)((CSD_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
	cardinfo->SD_csd.Reserved1 = tmp & 0x03;
	
	/* Byte 1 */
	tmp = (uint8_t)((CSD_Tab[0] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.TAAC = tmp;
	
	/* Byte 2 */
	tmp = (uint8_t)((CSD_Tab[0] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.NSAC = tmp;
	
	/* Byte 3 */
	tmp = (uint8_t)(CSD_Tab[0] & 0x000000FF);
	cardinfo->SD_csd.MaxBusClkFrec = tmp;
	
	/* Byte 4 */
	tmp = (uint8_t)((CSD_Tab[1] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CardComdClasses = tmp << 4;
	
	/* Byte 5 */
	tmp = (uint8_t)((CSD_Tab[1] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
	cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;
	
	/* Byte 6 */
	tmp = (uint8_t)((CSD_Tab[1] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.Reserved2 = 0; /* Reserved */
	
	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || \
		(CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;
	
		/* Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
	
		/* Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
	
		/* Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;

		/* Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
	
		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;
	}
	else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		/* Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;
	
		/* Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
	
		cardinfo->SD_csd.DeviceSize |= (tmp << 8);
	
		/* Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
	
		cardinfo->SD_csd.DeviceSize |= (tmp);
	
		/* Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
	
		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
		cardinfo->CardBlockSize = 512;    
	}
	
	cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;
	
	/* Byte 11 */
	tmp = (uint8_t)(CSD_Tab[2] & 0x000000FF);
	cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);
	
	/* Byte 12 */
	tmp = (uint8_t)((CSD_Tab[3] & 0xFF000000) >> 24);
	cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
	cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
	cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;
	
	/* Byte 13 */
	tmp = (uint8_t)((CSD_Tab[3] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.Reserved3 = 0;
	cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);
	
	/* Byte 14 */
	tmp = (uint8_t)((CSD_Tab[3] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
	cardinfo->SD_csd.ECC = (tmp & 0x03);
	
	/* Byte 15 */
	tmp = (uint8_t)(CSD_Tab[3] & 0x000000FF);
	cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_csd.Reserved4 = 1;
	
	/* Byte 0 */
	tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ManufacturerID = tmp;
	
	/* Byte 1 */
	tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.OEM_AppliID = tmp << 8;
	
	/* Byte 2 */
	tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8);
	cardinfo->SD_cid.OEM_AppliID |= tmp;
	
	/* Byte 3 */
	tmp = (uint8_t)(CID_Tab[0] & 0x000000FF);
	cardinfo->SD_cid.ProdName1 = tmp << 24;
	
	/* Byte 4 */
	tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdName1 |= tmp << 16;
	
	/* Byte 5 */
	tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.ProdName1 |= tmp << 8;
	
	/* Byte 6 */
	tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ProdName1 |= tmp;
	
	/* Byte 7 */
	tmp = (uint8_t)(CID_Tab[1] & 0x000000FF);
	cardinfo->SD_cid.ProdName2 = tmp;
	
	/* Byte 8 */
	tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdRev = tmp;
	
	/* Byte 9 */
	tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.ProdSN = tmp << 24;
	
	/* Byte 10 */
	tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ProdSN |= tmp << 16;
	
	/* Byte 11 */
	tmp = (uint8_t)(CID_Tab[2] & 0x000000FF);
	cardinfo->SD_cid.ProdSN |= tmp << 8;
	
	/* Byte 12 */
	tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24);
	cardinfo->SD_cid.ProdSN |= tmp;
	
	/* Byte 13 */
	tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16);
	cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
	cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;
	
	/* Byte 14 */
	tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
	cardinfo->SD_cid.ManufactDate |= tmp;
	
	/* Byte 15 */
	tmp = (uint8_t)(CID_Tab[3] & 0x000000FF);
	cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_cid.Reserved2 = 1;
	
	return(errorstatus);
}	
	
/**
 * @brief	Get SD card status.
 * @param	cardstatus: pointer to the SD_CardStatus structure
 * @retval	SD_Error
 */
SD_Error SDC_GetCardStatus(SD_CardStatus *cardstatus)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;
	
	errorstatus = SDC_SendSDStatus((uint32_t *)SDSTATUS_Tab);
	
	if (errorstatus  != SD_OK)
	{
		return(errorstatus);
	}
	
	/* Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0xC0) >> 6);
	cardstatus->DAT_BUS_WIDTH = tmp;
	
	/* Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0x20) >> 5);
	cardstatus->SECURED_MODE = tmp;
	
	/* Byte 2 */
	tmp = (uint8_t)((SDSTATUS_Tab[2] & 0xFF));
	cardstatus->SD_CARD_TYPE = tmp << 8;
	
	/* Byte 3 */
	tmp = (uint8_t)((SDSTATUS_Tab[3] & 0xFF));
	cardstatus->SD_CARD_TYPE |= tmp;
	
	/* Byte 4 */
	tmp = (uint8_t)(SDSTATUS_Tab[4] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;
	
	/* Byte 5 */
	tmp = (uint8_t)(SDSTATUS_Tab[5] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;
	
	/* Byte 6 */
	tmp = (uint8_t)(SDSTATUS_Tab[6] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;
	
	/* Byte 7 */
	tmp = (uint8_t)(SDSTATUS_Tab[7] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;
	
	/* Byte 8 */
	tmp = (uint8_t)((SDSTATUS_Tab[8] & 0xFF));
	cardstatus->SPEED_CLASS = tmp;
	
	/* Byte 9 */
	tmp = (uint8_t)((SDSTATUS_Tab[9] & 0xFF));
	cardstatus->PERFORMANCE_MOVE = tmp;
	
	/* Byte 10 */
	tmp = (uint8_t)((SDSTATUS_Tab[10] & 0xF0) >> 4);
	cardstatus->AU_SIZE = tmp;
	
	/* Byte 11 */
	tmp = (uint8_t)(SDSTATUS_Tab[11] & 0xFF);
	cardstatus->ERASE_SIZE = tmp << 8;
	
	/* Byte 12 */
	tmp = (uint8_t)(SDSTATUS_Tab[12] & 0xFF);
	cardstatus->ERASE_SIZE |= tmp;
	
	/* Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0xFC) >> 2);
	cardstatus->ERASE_TIMEOUT = tmp;
	
	/* Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0x3));
	cardstatus->ERASE_OFFSET = tmp;
	
	return(errorstatus);
	
}	

/**
 * @brief	Enables wide bus opeartion for the requeseted.
 * @param	WideMode: Specifies the SD card wide bus mode. 
 *   This parameter can be one of the following values:
 *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
 *     @arg SDIO_BusWide_4b: 4-bit data transfer
 *     @arg SDIO_BusWide_1b: 1-bit data transfer
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_EnableWideBusOperation(uint32_t WideMode)
{
	SD_Error errorstatus = SD_OK;
	
	/* MMC Card doesn't support this feature */
	if (SDIO_MULTIMEDIA_CARD == CardType)
	{
		errorstatus = SD_UNSUPPORTED_FEATURE;
		return(errorstatus);
	}
	else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || \
		(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || \
		(SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		if (SDIO_BusWide_8b == WideMode)
		{
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return(errorstatus);
		}
		else if (SDIO_BusWide_4b == WideMode)
		{
			errorstatus = SDC_EnWideBus(ENABLE);
	
		if (SD_OK == errorstatus)
		{
			/* Configure the SDIO peripheral */
			MCU_SDIOInit(SDIO_TRNS_CLK_DIV, SDIO_BusWide_4b);
		}
		}
		else
		{
			errorstatus = SDC_EnWideBus(DISABLE);
		
			if (SD_OK == errorstatus)
			{
				/* Configure the SDIO peripheral */
				MCU_SDIOInit(SDIO_TRNS_CLK_DIV, SDIO_BusWide_1b);
			}
		}
	}
	
	return(errorstatus);
}	
	
/**
 * @brief	Selects od Deselects the corresponding card.
 * @param	addr: Address of the Card to be selected.
 * @retval	SD_Error: SD Card Error code.
 */
SD_Error SDC_SelectDeselect(uint32_t addr)
{
	SD_Error errorstatus = SD_OK;
	
	/* Send CMD7 SDIO_SEL_DESEL_CARD */
	SDIO_CmdInitStructure.SDIO_Argument =  addr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
	
	return(errorstatus);
}

/**
 * @brief  Allows to read one block from a specified address in a card. The Data
 *         transfer can be managed by DMA mode or Polling mode. 
 * @note   This operation should be followed by two functions to check if the 
 *         DMA Controller and SD Card status.
 *          - SDC_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SDC_GetStatus(): to check that the SD Card has finished the 
 *            data transfer and it is ready for data.            
 * @param  readbuff: pointer to the buffer that will contain the received data
 * @param  ReadAddr: Address from where data are to be read.  
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
#if defined(SD_POLLING_MODE) 
	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;
	SDIO->DCTRL = 0x0;
	
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}
	
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	
	/* Send CMD17 READ_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

#if defined(SD_POLLING_MODE)  

	/* read 1 block of data by polling */
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | \
		SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			/* FIXME: Speed optimization of following code might make 
			 * it possible to use 4bit bus transaction.
			 */
			for (count = 0; count < 8; count++)
			{
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += 8;
		}
	}
	
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	/* read any additional data left */
	while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
	{
		*tempbuff = SDIO_ReadData();
		tempbuff++;
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/* In case of single block transfer, no need of stop transfer at all.*/

#elif defined(SD_DMA_MODE)

    SDIO_ITConfig(SDIO_IT_DATAEND, ENABLE);
    SDC_DMARxConfig((uint32_t *)readbuff, BlockSize);
    SDIO_DMACmd(ENABLE);

#endif

	return(errorstatus);
}

/**
 * @brief  Allows to read blocks from a specified address  in a card.  The Data
 *         transfer can be managed by DMA mode or Polling mode. 
 * @note   This operation should be followed by two functions to check if the 
 *         DMA Controller and SD Card status.
 *          - SDC_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SDC_GetStatus(): to check that the SD Card has finished the 
 *            data transfer and it is ready for data.   
 * @param  readbuff: pointer to the buffer that will contain the received data.
 * @param  ReadAddr: Address from where data are to be read.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @param  NumberOfBlocks: number of blocks to be read.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
#if defined(SD_POLLING_MODE)
	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;
	
	SDIO->DCTRL = 0x0;
	
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}
	
	/* Set Block Size for Card */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	
	if (SD_OK != errorstatus)
	{
		return(errorstatus);
	}
    
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	
	/* Send CMD18 READ_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
#if defined(SD_POLLING_MODE) 

	/* read data by polling */
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | \
		SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DATAEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			/* FIXME: Speed optimization of following code might make 
			 * it possible to use 4bit bus transaction.
			 */
			for (count = 0; count < 8; count++)
			{
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += 8;
		}
	}
	
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	/* read any additional data left */
	while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
	{
		*tempbuff = SDIO_ReadData();
		tempbuff++;
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/* send stop transfer command */
	errorstatus = SDC_StopTransfer();

#else

	SDIO_ITConfig(SDIO_IT_DATAEND, ENABLE);
	SDC_DMARxConfig((uint32_t *)readbuff, (NumberOfBlocks * BlockSize));
	SDIO_DMACmd(ENABLE);

#endif /* SD_POLLING_MODE */

	return(errorstatus);
}

/**
 * @brief  This function waits until the SDIO DMA data transfer is finished. 
 *         This function should be called after SDIO_ReadMultiBlocks() function
 *         to insure that all data sent by the card are already transferred by 
 *         the DMA controller.        
 * @param  None.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_WaitReadOperation(void)
{

#if defined(SD_DMA_MODE)

	while ((SDC_DMAEndOfTransferStatus() == RESET) && 
		(TransferEnd == 0) && (TransferError == SD_OK)) {}
	
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}

#endif // SD_DMA_MODE
	
	return(SD_OK);
}

/**
 * @brief  Allows to write one block starting from a specified address in a card.
 *         The Data transfer can be managed by DMA mode or Polling mode.
 * @note   This operation should be followed by two functions to check if the 
 *         DMA Controller and SD Card status.
 *          - SDC_ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SDC_GetStatus(): to check that the SD Card has finished the 
 *            data transfer and it is ready for data.      
 * @param  writebuff: pointer to the buffer that contain the data to be transferred.
 * @param  WriteAddr: Address from where data are to be read.   
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_WriteBlock(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;

#if defined(SD_POLLING_MODE)
	uint32_t bytestransferred = 0, count = 0, restwords = 0;
	uint32_t *tempbuff = (uint32_t *)writebuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;
	SDIO->DCTRL = 0x0;
	
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}
  
	/* Send CMD24 WRITE_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

#if defined(SD_POLLING_MODE) 

	while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | \
		SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
		{
			/* FIXME: Speed optimization of following code might make 
			 * it possible to use 4bit bus transaction.
			 */
			if ((512 - bytestransferred) < 32)
			{
				restwords = ((512 - bytestransferred) % 4 == 0) ? \
					((512 - bytestransferred) / 4) : (( 512 -  bytestransferred) \
					/ 4 + 1);
				for (count = 0; count < restwords; count++, tempbuff++, \
					bytestransferred += 4)
				{
					SDIO_WriteData(*tempbuff);
				}
			}
			else
			{
				for (count = 0; count < 8; count++)
				{
					SDIO_WriteData(*(tempbuff + count));
				}
				tempbuff += 8;
				bytestransferred += 32;
			}
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
		errorstatus = SD_TX_UNDERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

	/* In case of single data block transfer no need of stop command at all */

#elif defined(SD_DMA_MODE)

	SDIO_ITConfig(SDIO_IT_DATAEND, ENABLE);
	SDC_DMATxConfig((uint32_t *)writebuff, BlockSize);
	SDIO_DMACmd(ENABLE);

#endif

	return(errorstatus);
}

/**
 * @brief  Allows to write blocks starting from a specified address in a card.
 *         The Data transfer can be managed by DMA mode only. 
 * @note   This operation should be followed by two functions to check if the 
 *         DMA Controller and SD Card status.
 *          - SDC__ReadWaitOperation(): this function insure that the DMA
 *            controller has finished all data transfer.
 *          - SDC_GetStatus(): to check that the SD Card has finished the 
 *            data transfer and it is ready for data.     
 * @param  WriteAddr: Address from where data are to be read.
 * @param  writebuff: pointer to the buffer that contain the data to be transferred.
 * @param  BlockSize: the SD card Data block size. The Block size should be 512.
 * @param  NumberOfBlocks: number of blocks to be written.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_WriteMultiBlocks(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	
#if defined(SD_POLLING_MODE)
	uint32_t bytestransferred = 0, count = 0, restwords = 0, datalength;
	uint32_t *tempbuff = (uint32_t *)writebuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 1;
	SDIO->DCTRL = 0x0;
	
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}
	
	/* To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	/* To improve performance */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)NumberOfBlocks;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	/* Send CMD25 WRITE_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);
	
	if (SD_OK != errorstatus)
	{
		return(errorstatus);
	}
	
	datalength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = datalength;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	
#if defined(SD_POLLING_MODE)

	while (!(SDIO->STA & (SDIO_FLAG_DATAEND | SDIO_FLAG_TXUNDERR | \
		SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
		{
			/* FIXME: Speed optimization of following code might make 
			 * it possible to use 4bit bus transaction.
			 */
			if ((datalength - bytestransferred) < 32)
			{
				restwords = ((datalength - bytestransferred) % 4 == 0) ? \
					((datalength - bytestransferred) / 4) : (( datalength -  bytestransferred) \
					/ 4 + 1);
				for (count = 0; count < restwords; count++, tempbuff++, \
					bytestransferred += 4)
				{
					SDIO_WriteData(*tempbuff);
				}
			}
			else
			{
				for (count = 0; count < 8; count++)
				{
					SDIO_WriteData(*(tempbuff + count));
				}
				tempbuff += 8;
				bytestransferred += 32;
			}
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
		errorstatus = SD_TX_UNDERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

	/* send stop transfer command */
	errorstatus = SDC_StopTransfer();

#else

	SDIO_ITConfig(SDIO_IT_DATAEND, ENABLE);
	SDC_DMATxConfig((uint32_t *)writebuff, (NumberOfBlocks * BlockSize));
	SDIO_DMACmd(ENABLE);    

#endif
	
	return(errorstatus);
}

/**
 * @brief  This function waits until the SDIO DMA data transfer is finished. 
 *         This function should be called after SDIO_WriteBlock() and
 *         SDIO_WriteMultiBlocks() function to insure that all data sent by the 
 *         card are already transferred by the DMA controller.        
 * @param  None.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_WaitWriteOperation(void)
{

#if defined(SD_DMA_MODE)
	
	while ((SDC_DMAEndOfTransferStatus() == RESET) && \
		(TransferEnd == 0) && (TransferError == SD_OK)) {}
	
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
#endif // SD_DMA_MODE

	return(SD_OK);
}

/**
 * @brief  Gets the cuurent data transfer state.
 * @param  None
 * @retval SDTransferState: Data Transfer state.
 *   This value can be: 
 *        - SD_TRANSFER_OK: No data transfer is acting
 *        - SD_TRANSFER_BUSY: Data transfer is acting
 */
SDTransferState SDC_GetTransferState(void)
{
	if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
	{
		return(SD_TRANSFER_BUSY);
	}
	else
	{
		return(SD_TRANSFER_OK);
	}
}

/**
 * @brief  Aborts an ongoing data transfer.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_StopTransfer(void)
{
	SD_Error errorstatus = SD_OK;
	
	/* Send CMD12 STOP_TRANSMISSION  */
	SDIO->ARG = 0x0;
	SDIO->CMD = 0x44C;
	errorstatus = SDC_CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
	
	return(errorstatus);
}

/**
 * @brief  Allows to erase memory area specified for the given card.
 * @param  startaddr: the start address.
 * @param  endaddr: the end address.
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_Erase(uint32_t startaddr, uint32_t endaddr)
{
	SD_Error errorstatus = SD_OK;
	uint32_t delay = 0;
	__IO uint32_t maxdelay = 0;
	uint8_t cardstate = 0;
	
	/* Check if the card coomnd class supports erase command */
	if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return(errorstatus);
	}
	
	maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);
	
	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}
	
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		startaddr /= 512;
		endaddr /= 512;
	}
	
	/* According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and 
		erase_group_end(CMD33) */
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || \
		(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || \
		(SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/* Send CMD32 SD_ERASE_GRP_START with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = startaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_START;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
	
		errorstatus = SDC_CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	
		/* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = endaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_END;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
	
		errorstatus = SDC_CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);

		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}
	
	/* Send CMD38 ERASE */
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ERASE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = SDC_CmdResp1Error(SD_CMD_ERASE);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	for (delay = 0; delay < maxdelay; delay++) {}
	
	/* Wait till the card is in programming state */
	errorstatus = SDC_IsCardProgramming(&cardstate);
	
	while ((errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || \
		(SD_CARD_RECEIVING == cardstate)))
	{
		errorstatus = SDC_IsCardProgramming(&cardstate);
	}
	
	return(errorstatus);
}

/**
 * @brief  Returns the current card's status.
 * @param  pcardstatus: pointer to the buffer that will contain the SD card 
 *         status (Card Status register).
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;
	
	SDIO->ARG = (uint32_t) RCA << 16;
	SDIO->CMD = 0x44D;
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SEND_STATUS);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	*pcardstatus = SDIO->RESP1;
	return(errorstatus);
}

/**
 * @brief  Returns the current SD card's status.
 * @param  psdstatus: pointer to the buffer that will contain the SD card status 
 *         (SD Status register).
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_SendSDStatus(uint32_t *psdstatus)
{
	SD_Error errorstatus = SD_OK;
	uint32_t count = 0;
	
	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	/* Set block size for card if it is not equal to current block size for card. */
	SDIO_CmdInitStructure.SDIO_Argument = 64;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/* CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 64;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	
	/* Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_STAUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = SDC_CmdResp1Error(SD_CMD_SD_APP_STAUS);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | \
		SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			for (count = 0; count < 8; count++)
			{
				*(psdstatus + count) = SDIO_ReadData();
			}
			psdstatus += 8;
		}
	}
	
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	
	while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
	{
		*psdstatus = SDIO_ReadData();
		psdstatus++;
	}
	
	/* Clear all the static status flags*/
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	return(errorstatus);
}

/**
 * @brief  Allows to process all the interrupts that are high.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
SD_Error SDC_ProcessIRQSrc(void)
{
	if (StopCondition == 1)
	{
		SDIO->ARG = 0x0;
		SDIO->CMD = 0x44C;
		TransferError = SDC_CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
	}
	else
	{
		TransferError = SD_OK;
	}

	SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
	SDIO_ITConfig(SDIO_IT_DATAEND, DISABLE);
	TransferEnd = 1;

	return(TransferError);
}

/**
 * @brief  Checks for error conditions for CMD0.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;
	
	timeout = SDIO_CMD0TIMEOUT; /* 10000 */
	
	while ((timeout > 0) && (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET))
	{
		timeout--;
	}
	
	if (timeout == 0)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		return(errorstatus);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	return(errorstatus);
}

/**
 * @brief  Checks for error conditions for R7 response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdResp7Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t timeout = SDIO_CMD0TIMEOUT;
	
	status = SDIO->STA;
	
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | 
		SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
	{
		timeout--;
		status = SDIO->STA;
	}
	
	if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
	{
		/* Card is not V2.0 complient or card does not support the set 
			voltage range */
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	
	if (status & SDIO_FLAG_CMDREND)
	{
		/* Card is SD V2.0 compliant */
		errorstatus = SD_OK;
		SDIO_ClearFlag(SDIO_FLAG_CMDREND);
		return(errorstatus);
	}
	return(errorstatus);
}

/**
 * @brief  Checks for error conditions for R1 response.
 * @param  cmd: The sent command index.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdResp1Error(uint8_t cmd)
{
	while (!(SDIO->STA & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | 
		SDIO_FLAG_CTIMEOUT))) { }
	
	SDIO->ICR = SDIO_STATIC_FLAGS;
	
	return (SD_Error)(SDIO->RESP1 &  SD_OCR_ERRORBITS);
}

/**
 * @brief  Checks for error conditions for R3 (OCR) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdResp3Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	
	status = SDIO->STA;
	
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | 
		SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
	}
	
	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}

	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	return(errorstatus);
}

/**
 * @brief  Checks for error conditions for R2 (CID or CSD) response.
 * @param  None
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdResp2Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	
	status = SDIO->STA;
	
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | \
		SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
	}
	
	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	return(errorstatus);
}

/**
 * @brief  Checks for error conditions for R6 (RCA) response.
 * @param  cmd: The sent command index.
 * @param  prca: pointer to the variable that will contain the SD card relative 
 *         address RCA. 
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;
	
	status = SDIO->STA;
	
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | \
		SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
	}
	
	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}
	
	/* Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return(errorstatus);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	/* We have received response, retrieve it.  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);
	
	if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | \
		SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
	{
		*prca = (uint16_t) (response_r1 >> 16);
		return(errorstatus);
	}
	
	if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
	{
		return(SD_GENERAL_UNKNOWN_ERROR);
	}
	
	if (response_r1 & SD_R6_ILLEGAL_CMD)
	{
		return(SD_ILLEGAL_CMD);
	}
	
	if (response_r1 & SD_R6_COM_CRC_FAILED)
	{
		return(SD_COM_CRC_FAILED);
	}
	
	return(errorstatus);
}

/**
 * @brief  Enables or disables the SDIO wide bus mode.
 * @param  NewState: new state of the SDIO wide bus mode.
 *   This parameter can be: ENABLE or DISABLE.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_EnWideBus(FunctionalState NewState)
{
	SD_Error errorstatus = SD_OK;
	
	uint32_t scr[2] = {0, 0};
	
	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}
	
	/* Get SCR Register */
	errorstatus = SDC_FindSCR(RCA, scr);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	/* If wide bus operation to be enabled */
	if (NewState == ENABLE)
	{
		/* If requested card supports wide bus operation */
		if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/* Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
			errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
	
			/* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x2;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
			errorstatus = SDC_CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
	
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			return(errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return(errorstatus);
		}
	}   /* If wide bus operation to be disabled */
	else
	{
		/* If requested card supports 1 bit mode operation */
		if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/* Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	
			errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
	
			/* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
	
			errorstatus = SDC_CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
	
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
	
			return(errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return(errorstatus);
		}
	}
}

/**
 * @brief  Checks if the SD card is in programming state.
 * @param  pstatus: pointer to the variable that will contain the SD card state.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_IsCardProgramming(uint8_t *pstatus)
{
	SD_Error errorstatus = SD_OK;
	__IO uint32_t respR1 = 0, status = 0;
	
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	status = SDIO->STA;
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | \
		SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
	}
	
	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}
	
	status = (uint32_t)SDIO_GetCommandResponse();
	
	/* Check response received is of desired command */
	if (status != SD_CMD_SEND_STATUS)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return(errorstatus);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	
	/* We have received response, retrieve it for analysis  */
	respR1 = SDIO_GetResponse(SDIO_RESP1);
	
	/* Find out card status */
	*pstatus = (uint8_t) ((respR1 >> 9) & 0x0000000F);
	
	if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
	{
		return(errorstatus);
	}
	
	if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
	{
		return(SD_ADDR_OUT_OF_RANGE);
	}
	
	if (respR1 & SD_OCR_ADDR_MISALIGNED)
	{
		return(SD_ADDR_MISALIGNED);
	}
	
	if (respR1 & SD_OCR_BLOCK_LEN_ERR)
	{
		return(SD_BLOCK_LEN_ERR);
	}
	
	if (respR1 & SD_OCR_ERASE_SEQ_ERR)
	{
		return(SD_ERASE_SEQ_ERR);
	}
	
	if (respR1 & SD_OCR_BAD_ERASE_PARAM)
	{
		return(SD_BAD_ERASE_PARAM);
	}

	if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
	{
		return(SD_WRITE_PROT_VIOLATION);
	}
	
	if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
	{
		return(SD_LOCK_UNLOCK_FAILED);
	}
	
	if (respR1 & SD_OCR_COM_CRC_FAILED)
	{
		return(SD_COM_CRC_FAILED);
	}
	
	if (respR1 & SD_OCR_ILLEGAL_CMD)
	{
		return(SD_ILLEGAL_CMD);
	}
	
	if (respR1 & SD_OCR_CARD_ECC_FAILED)
	{
		return(SD_CARD_ECC_FAILED);
	}
	
	if (respR1 & SD_OCR_CC_ERROR)
	{
		return(SD_CC_ERROR);
	}
	
	if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
	{
		return(SD_GENERAL_UNKNOWN_ERROR);
	}
	
	if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
	{
		return(SD_STREAM_READ_UNDERRUN);
	}
	
	if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
	{
		return(SD_STREAM_WRITE_OVERRUN);
	}
	
	if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
	{
		return(SD_CID_CSD_OVERWRITE);
	}
	
	if (respR1 & SD_OCR_WP_ERASE_SKIP)
	{
		return(SD_WP_ERASE_SKIP);
	}
	
	if (respR1 & SD_OCR_CARD_ECC_DISABLED)
	{
		return(SD_CARD_ECC_DISABLED);
	}
	
	if (respR1 & SD_OCR_ERASE_RESET)
	{
		return(SD_ERASE_RESET);
	}
	
	if (respR1 & SD_OCR_AKE_SEQ_ERROR)
	{
		return(SD_AKE_SEQ_ERROR);
	}
	
	return(errorstatus);
}

/**
 * @brief  Find the SD card SCR register value.
 * @param  rca: selected card address.
 * @param  pscr: pointer to the buffer that will contain the SCR value.
 * @retval SD_Error: SD Card Error code.
 */
static SD_Error SDC_FindSCR(uint16_t rca, uint32_t *pscr)
{
	uint32_t index = 0;
	SD_Error errorstatus = SD_OK;
	uint32_t tempscr[2] = {0, 0};
	
	/* Set Block Size To 8 Bytes */
	/* Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	/* Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_APP_CMD);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 8;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	
	
	/* Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	
	errorstatus = SDC_CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
	
	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	
	while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | \
		SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
		{
			*(tempscr + index) = SDIO_ReadData();
			index++;
		}
	}
	
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	*(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | \
		((tempscr[0] & SD_8TO15BITS) << 8) | \
		((tempscr[0] & SD_16TO23BITS) >> 8) | \
		((tempscr[0] & SD_24TO31BITS) >> 24);
	
	*(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | \
		((tempscr[1] & SD_8TO15BITS) << 8) | \
		((tempscr[1] & SD_16TO23BITS) >> 8) | \
		((tempscr[1] & SD_24TO31BITS) >> 24);
	
	return(errorstatus);
}

/*
 * @brief	Set block size of SD card
 * @param	BlockSize
 * @retval	SD_Error
 */
SD_Error SDC_SetBlockSize(uint32_t BlockSize)
{
	SDIO_CmdInitStructure.SDIO_Argument = BlockSize;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	return( SDC_CmdResp1Error(SD_CMD_SET_BLOCKLEN) );

}


/* END OF FILE */
