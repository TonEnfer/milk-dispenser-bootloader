#include "config.h"
#include "stm32h7xx_hal.h"


#define BAD_CRC 0xBADCEDAF

__attribute__ ((section(".eeprom"))) struct tBootloaderConfig appConfig = {
		.application_runs = 0,
		.application_crc = BAD_CRC,
		.wait_for_flash_activation_confirmation_ms = 3000,
		.activate_flash_drive_timeout_ms = 3000,
		.activate_main_program_timeout_ms = 3000,
};

static struct tBootloaderConfig appConfigRAM;


static void loadAppAconfig(){
	appConfig = appConfigRAM;
}

static void saveAppConfig(){
	HAL_FLASH_Unlock();
	uint32_t appConfigAddressStart = (uint32_t)&appConfig;
	uint32_t appConfigSize = sizeof(appConfig);
	uint32_t appConfigAddressStartRAM = (uint32_t)&appConfigRAM;
	for(uint32_t shift = 0; shift<appConfigSize; shift++){
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, appConfigAddressStart+shift,  appConfigAddressStartRAM+shift);
	}
	HAL_FLASH_Lock();
}

uint64_t AppConfig_incrementRuns(){
	loadAppAconfig();
	appConfigRAM.application_runs++;
	saveAppConfig();
	return appConfigRAM.application_runs;
}

