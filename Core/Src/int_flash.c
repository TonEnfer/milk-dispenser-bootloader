#include <string.h>

#include "int_flash.h"
#include "log.h"

#define INT_FLASH_LOG

#ifdef INT_FLASH_LOG
#define IFLOG(X) X
#else
#define IFLOG(X)
#endif

#define FLASH_WORD_SIZE 0x20UL

#define FLASH_SECTOR_MASK     0xFFFE0000UL
#define FLASH_SECTOR_IN_MASK  0x0001FFFFUL
#define FLASH_ADDRESS_IN_MASK 0x001FFFFFUL

#define SECTOR_NUMBER_SHIFT 17UL
#define SECTOR_NUMBER_MASK (7UL << SECTOR_NUMBER_SHIFT)

#define _NOT_A_SECTOR 0xFFFFFFFFUL

static uint32_t _CURRENT_SECTOR = _NOT_A_SECTOR;

static uint32_t _SECTOR_DATA[FLASH_SECTOR_SIZE/sizeof(uint32_t)] = {0};
static uint8_t* _SECTOR_DATA_8 = (uint8_t *)_SECTOR_DATA;

static uint32_t _sector_number(uint32_t address){
	return (address & SECTOR_NUMBER_MASK) >> SECTOR_NUMBER_SHIFT;
}


HAL_StatusTypeDef IntFlash_unlock(){
	HAL_StatusTypeDef status;
	if(HAL_OK!=(status = HAL_FLASH_Unlock())){
		return status;
	}
	return HAL_OK;
}

static void _flash_err(){
	log_error("Flash error: 0x%02X", HAL_FLASH_GetError());
}

bool _erase_sector(uint32_t sector){
	IFLOG(log_info(">_erase_sector %d", sector));

	if(sector <2){
		log_error("Trying to erase bootloader");
		return false;
	}

	HAL_FLASHEx_Unlock_Bank1();
	HAL_FLASHEx_Unlock_Bank2();
	HAL_StatusTypeDef status;
	if(HAL_OK!=(status = IntFlash_unlock())){
		log_error("IntFlash_erase_app IntFlash_unlock error: 0x%02X", status);
		_flash_err();
		return false;
	}

	FLASH_EraseInitTypeDef erase;
	erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase.Banks = FLASH_BANK_1;
	erase.Sector = sector;
	erase.NbSectors = 1;
	erase.VoltageRange = FLASH_VOLTAGE_RANGE_1;
	uint32_t faultySector = 0xFFFFFFFF;
	status = HAL_FLASHEx_Erase(&erase, &faultySector);
	if(HAL_OK!=status){
		log_error("IntFlash_erase_app HAL_FLASHEx_Erase error: 0x%02X", status);
		_flash_err();
		return false;
	}

	IFLOG(log_info("<_erase_sector %d", sector));
	return true;
}

static void _load_sector(uint32_t address){
	IFLOG(log_info(">_load_sector 0x%08X", address));
	uint32_t new_sector = address & FLASH_SECTOR_MASK;
	__IO uint8_t* flash_iter = (__IO uint8_t*)(address & FLASH_SECTOR_MASK);
	for(uint32_t i=0; i<FLASH_SECTOR_SIZE; i++){
		_SECTOR_DATA_8[i] = flash_iter[i];
	}
	_CURRENT_SECTOR = new_sector;
	IFLOG(log_info("<_load_sector 0x%08X", address));
}

static bool _in_current_sector(uint32_t address){
	return _CURRENT_SECTOR == (address & FLASH_SECTOR_MASK);
}

static bool _write_byte(uint32_t address, uint8_t byte){
	// flash word is 32 bytes. 31 bytes will give use
	if(!_in_current_sector(address)){
		IntFlash_sync();
		_load_sector(address);
	}
	uint32_t inSectorAddress = address & FLASH_SECTOR_IN_MASK;
	_SECTOR_DATA_8[inSectorAddress] = byte;
	return true;
}

bool IntFlash_write(uint32_t address, uint8_t *buff, size_t bw){
	for (size_t i=0; i<bw; i++){
		if(!_write_byte(address+i, buff[i])){
			return false;
		}
	}
	return true;
}

static bool _need_sync(){
	IntFlash_unlock();
	if(_CURRENT_SECTOR == _NOT_A_SECTOR){
		return false;
	}
	__IO uint8_t* flash_iter = (__IO uint8_t*)(_CURRENT_SECTOR & FLASH_SECTOR_MASK);
	for(uint32_t i=0; i<FLASH_SECTOR_SIZE; i++){
		if(_SECTOR_DATA_8[i] != flash_iter[i]){
			return true;
		}
	}
	IFLOG(log_info("No need to sync sector 0x%08X", _CURRENT_SECTOR));
	return false;
}

static bool _sync_sector(){
	if(!_erase_sector(_sector_number(_CURRENT_SECTOR))){
		return false;
	}

	for(uint32_t i=0; i<FLASH_SECTOR_SIZE; i+=FLASH_WORD_SIZE){
		HAL_StatusTypeDef status;
		if(HAL_OK!=(status = IntFlash_unlock())){
			log_error("IntFlash_sync IntFlash_unlock error: 0x%02X", status);
			_flash_err();
			return false;
		}

		uint32_t flash_address = _CURRENT_SECTOR + i;
		uint32_t data_address = (uint32_t)(_SECTOR_DATA_8) +i;

		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_address, data_address);
		if(HAL_OK!=status){
			log_error("IntFlash_sync HAL_FLASH_Program error fa: 0x%02X, da: 0x%02X, err: 0x%02X", flash_address, data_address, status);
			_flash_err();
			return false;
		}
	}
	return true;
}

bool IntFlash_sync(){
	IFLOG(log_info(">IntFlash_sync"));
	if(!_need_sync()){
		IFLOG(log_info("<IntFlash_sync no sync needed"));
		return true;
	}

	if(!_sync_sector()){
		log_error("<IntFlash_sync sync failed");
		return false;
	}

	return !_need_sync();
}


#undef IFLOG
#undef INT_FLASH_LOG
