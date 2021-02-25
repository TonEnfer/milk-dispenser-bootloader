#include "int_flash.h"
#include "stm32h7xx_hal.h"

bool IntFlash_erase_app(){
	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef erase;
	erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase.Banks = FLASH_BANK_1;
	erase.Sector = 2;
	erase.NbSectors = 6;
	erase.VoltageRange = FLASH_VOLTAGE_RANGE_1;
	uint32_t faultySector = 0xFFFFFFFF;
	HAL_FLASHEx_Erase(&erase, &faultySector);
	HAL_FLASH_Lock();

	return true;
}

bool IntFlash_write(uint32_t address, uint8_t *buff, size_t bw){
	return false;
}

bool IntFlash_sync(){
	return false;
}

