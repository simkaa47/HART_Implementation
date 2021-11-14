#include "flash.h"
#include "main.h"

#define MYADDR 0x0800FC00;
//begin of  private variables
static FLASH_EraseInitTypeDef EraseInitStruct; // erase struct
//end of private variables

//begin of  external variables
extern uint8_t hart_address;
//end of external variables

// begin function prototips
void FlashInit(void);
bool FlashErase(void);
bool FlashWrite(void);
// end function prototips

// begin functions

// Запись во Flash
bool FlashWrite()
{
	uint32_t address = MYADDR;
	HAL_StatusTypeDef status;
	if(!FlashErase())return false;
	HAL_FLASH_Unlock();
	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, (uint16_t)hart_address);
	HAL_FLASH_Lock();
	return status==HAL_OK;
}

// Очистка памяти перед записью
bool FlashErase()
{
	HAL_StatusTypeDef status;
	uint32_t page_error = 0; 
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = MYADDR;
	EraseInitStruct.NbPages = 1;
	HAL_FLASH_Unlock();
	status = HAL_FLASHEx_Erase(&EraseInitStruct, &page_error);
	HAL_FLASH_Lock();
	return status==HAL_OK;
}

// Чтение даннх из Flash
void FlashInit()
{
	uint32_t adress = MYADDR;
	hart_address = (uint8_t)(*(uint16_t*)adress);
	if(hart_address==0xFF)hart_address=0;
}
// end functions