#include "config.h"

#define BAD_CRC 0xBADCEDAF

__attribute__ ((section(".eeprom"))) struct tBootloaderConfig appConfig = {
		.application_crc = BAD_CRC,
		.wait_for_flash_activation_confirmation_ms = 3000,
		.activate_flash_drive_timeout_ms = 3000,
		.activate_main_program_timeout_ms = 3000,
};
