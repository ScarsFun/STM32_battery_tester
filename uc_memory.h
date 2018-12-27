#ifndef UC_MEMORY_H
#define UC_MEMORY_H

	// CAN BE CHANGED
	#define FEE_DENSITY_PAGES	4	    // how many pages are used 
	#define FEE_PAGE_SIZE		1024	    // can be 1k or 2k check manual for used device
	#define FEE_PAGE_BASE_ADDRESS 	0x08012000  // choose location for the first EEPROMPage address on the top of flash

	// DONT CHANGE
	#define FEE_DENSITY_BYTES		((FEE_PAGE_SIZE / 2) * FEE_DENSITY_PAGES - 1)
	#define FEE_LAST_PAGE_ADDRESS 	(FEE_PAGE_BASE_ADDRESS + (FEE_PAGE_SIZE * FEE_DENSITY_PAGES))
	#define FEE_EMPTY_WORD			((uint16_t)0xFFFF)
	#define FEE_ADDR_OFFSET(Address)(Address * 2) // halfWord per Word will be saved to preserve Flash

	// use this function to initialize the functionality
	uint16_t FEE_Init(void);
	void FEE_Erase (void);
	uint16_t FEE_WriteDataHalfWord (uint16_t Address, uint16_t DataHalfWord);
	uint16_t FEE_ReadDataHalfWord (uint16_t Address);

#endif
