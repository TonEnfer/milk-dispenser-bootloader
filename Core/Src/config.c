#include "config.h"
#include "stm32h7xx_hal.h"
#include "ff.h"
#include <stdbool.h>
#include <string.h>
#define BAD_CRC 0xBADCEDAF

#define BTLDR_CFG "btldr.cfg"

struct tBootloaderConfigData {
	union uBootloaderConfig {
		struct tBootloaderConfig config;
		uint32_t data[(sizeof(struct tBootloaderConfig) + sizeof(uint32_t) - 1)
				/ sizeof(uint32_t)];
	} data;
	uint32_t crc;
};

struct tBootloaderConfigData appConfig;

static uint32_t crc(struct tBootloaderConfigData *config){
	const size_t datasize = sizeof(config->data.data)/sizeof(config->data.data[0]);

	uint32_t sum = 0;
	for (size_t i=0; i<datasize; i++){
		sum <<= 1;
		sum += config->data.data[i];
	}

	return sum;
}

static void makeDefaultConfig() {
	appConfig = (struct tBootloaderConfigData){
		.data = {
			.config = {
				.application_runs = 0, .application_crc = BAD_CRC,
				.wait_for_flash_activation_confirmation_ms = 3000,
				.activate_flash_drive_timeout_ms = 0,
				.activate_main_program_timeout_ms = 0,
			}
		},
		.crc = BAD_CRC
	};
	appConfig.crc = crc(&appConfig);
}

static bool checkConfigExists() {
	FILINFO finfo;
	if (FR_OK != f_stat(BTLDR_CFG, &finfo)) {

		return false;
	}
	if (!strlen(finfo.fname)) {
		return false;
	}
	return true;
}

static bool checkConfigSizeok(){
	FILINFO finfo;
	if (FR_OK != f_stat(BTLDR_CFG, &finfo)) {
		return false;
	}
	if(finfo.fsize!=sizeof(appConfig)){
		return false;
	}
	return true;
}

static bool readConfig(struct tBootloaderConfigData *tmp){
	FIL file;
	FRESULT fr;
	if(FR_OK!=(fr=f_open(&file, BTLDR_CFG, FA_READ))){
		f_close(&file);
		return false;
	}

	UINT bytes_read = 0;

	if(FR_OK!=(fr=f_read(&file, tmp, sizeof(struct tBootloaderConfigData), &bytes_read))){
		f_close(&file);
		return false;
	}

	if(FR_OK!=(fr=f_close(&file))){
		return false;
	}

	if(bytes_read!=sizeof(*tmp)){
		return false;
	}

	return true;
}

static bool checkConfigValid() {
	struct tBootloaderConfigData tmp = {};
	if(!readConfig(&tmp)){
		return false;
	}

	uint32_t sum = crc(&tmp);
	if(sum!=tmp.crc){
		return false;
	}

	return true;
}

static void loadAppAconfig() {
	if(checkConfigExists() && checkConfigSizeok() && checkConfigValid()){
		readConfig(&appConfig);
	}else if(checkConfigExists()){
		makeDefaultConfig();
	}else {
		makeDefaultConfig();
	}

}

static bool saveAppConfig() {
	appConfig.crc = crc(&appConfig);

	FIL file;
	FRESULT fr;
	if(FR_OK!=(fr=f_open(&file, BTLDR_CFG, FA_WRITE | FA_CREATE_ALWAYS))){
		f_close(&file);
		return false;
	}
	UINT bytes_written = 0;
	if(FR_OK!=f_write(&file, &appConfig, sizeof(appConfig), &bytes_written)){
		f_close(&file);
		return false;
	}

	f_close(&file);
	return true;
}

uint64_t BtldrConfig_incrementRuns() {
	loadAppAconfig();
	appConfig.data.config.application_runs++;
	saveAppConfig();
	return appConfig.data.config.application_runs;
}

struct tBootloaderConfig* BtldrConfig_Get(){
	loadAppAconfig();
	saveAppConfig();
	return &appConfig.data.config;
}
