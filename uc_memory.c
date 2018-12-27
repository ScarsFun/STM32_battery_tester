//#include <stdio.h>
#include <string.h>
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
#include "uc_memory.h"
/*****************************************************************************
 * Allows to use the internal flash to store non volatile data. To initialize
 * the functionality use the FEE_Init() function. Be sure that by reprogramming
 * of the controller just affected pages will be deleted. In other case the non
 * volatile data will be lost.
******************************************************************************/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/


/*****************************************************************************
*  Delete Flash Space used for user Data, deletes the whole space between
*  RW_PAGE_BASE_ADDRESS and the last uC Flash Page
******************************************************************************/
uint16_t FEE_Init(void) {
	// unlock flash
	FLASH_Unlock();

	// Clear Flags
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);

	return FEE_DENSITY_BYTES;
}
/*****************************************************************************
*  Erase the whole reserved Flash Space used for user Data
******************************************************************************/
void FEE_Erase (void) {

	int page_num = 0;

	// delete all pages from specified start page to the last page
	do {
		FLASH_ErasePage(FEE_PAGE_BASE_ADDRESS + (page_num * FEE_PAGE_SIZE));
		page_num++;
	} while (page_num < FEE_DENSITY_PAGES);
}
/*****************************************************************************
*  Writes once data byte to flash on specified address. If a byte is already
*  written, the whole page must be copied to a buffer, the byte changed and
*  the manipulated buffer written after PageErase.
*******************************************************************************/
uint16_t FEE_WriteDataHalfWord (uint16_t Address, uint16_t DataHalfWord) {

	FLASH_Status FlashStatus = FLASH_COMPLETE;

	uint32_t page;
	

	// exit if desired address is above the limit (e.G. under 2048 Bytes for 4 pages)
	if (Address > FEE_DENSITY_BYTES) {
		return 0;
	}

	// calculate which page is affected (Pagenum1/Pagenum2...PagenumN)
	page = (FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address)) & 0x00000FFF;

	if (page % FEE_PAGE_SIZE) page = page + FEE_PAGE_SIZE;
	page = (page / FEE_PAGE_SIZE) - 1;

	// if current data is 0xFF, the byte is empty, just overwrite with the new one
	if ((*(uint16_t*)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address))) == FEE_EMPTY_WORD) {

		FlashStatus = FLASH_ProgramHalfWord(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address), DataHalfWord);
	}
	// Error half word not empty
	else
		return 0; 
	return FlashStatus;
}
/*****************************************************************************
*  Read once data byte from a specified address.
*******************************************************************************/
uint16_t FEE_ReadDataHalfWord (uint16_t Address) {

	uint16_t DataHalfWord = 0xFFFF;

	// Get Byte from specified address
	DataHalfWord = (*(uint16_t*)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address)));

	return DataHalfWord;
}
