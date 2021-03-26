#include <stdbool.h>
#include <string.h>

#include "apploader.h"
#include "int_flash.h"
#include "ff.h"
#include "log.h"

#define APP_BIN_PATH "app.hex"

static bool firmwareExists() {
	FILINFO finfo;
	FRESULT fr;
	fr = f_stat(APP_BIN_PATH, &finfo);
	if (fr != FR_OK) {
		return false;
	}

	if (strlen(finfo.fname) == 0) {
		return false;
	}

	// rough validation to make sure it fits in memory
	if (finfo.fsize > (1024 + 512 + 256) * 1024) {
		return false;
	}

	return true;
}

static uint8_t hexToUint(char ch) {
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	}
	if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	}
	if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	}
	return 0xFF;
}

static bool hexPairToInt(char *ch, uint8_t *val) {

	uint8_t high = hexToUint(ch[0]);
	uint8_t low = hexToUint(ch[1]);

	if (high == 0xFF || low == 0xFF) {
		return false;
	}
	*val = high << 4 | low;
	return true;
}

#pragma pack(1)
struct tHexRowChars {
	char byteCount[2];
	char address[4];
	char recordType[2];
};
#pragma pack()

enum tHexRecordType {
	IHEX_DATA = 0,
	IHEX_EOF = 1,
	IHEX_EXT_SEG_ADDR = 2,
	IHEX_START_SEG_ADDR = 3,
	IHEX_EXT_LIN_ADDR = 4,
	IHEX_START_LIN_ADDR = 5
};

struct tHexRowParsed {
	uint8_t byteCount;
	uint16_t addess;
	enum tHexRecordType recordType;
};

static bool hexRowCharToParsed(struct tHexRowChars *source,
		struct tHexRowParsed *target) {
	uint8_t byteCount = 0;
	uint16_t addHigh = 0, addLow = 0;
	enum tHexRecordType recordType;

	if (!hexPairToInt(source->byteCount, &byteCount)) {
		return false;
	}

	if (!hexPairToInt((char*) source->address, (uint8_t*) &addHigh)) {
		return false;
	}
	if (!hexPairToInt((char*) (source->address + 2), (uint8_t*) &addLow)) {
		return false;
	}

	if (!hexPairToInt(source->recordType, &recordType)) {
		return false;
	}

	target->byteCount = byteCount;
	target->addess = addHigh << 8 | addLow;
	target->recordType = recordType;
	return true;
}

bool readUntilColon(FIL *fp) {
	char ch;
	UINT br;
	for (;;) {
		if (FR_OK != f_read(fp, &ch, 1, &br)) {
			return false;
		}
		if (br == 0) {
			return false;
		}
		if (ch == ':') {
			return true;
		}
	}
	return false;
}
enum iHexCallbackRes {
	IHEX_CB_OK, IHEX_CB_EOF, IHEX_CB_ERROR
};
static enum iHexCallbackRes iHexRowValidateChecksum(
		struct tHexRowParsed rowParsed, uint8_t *data, uint8_t checksum) {
	uint8_t sum = rowParsed.byteCount + (rowParsed.addess & 0xFF)
			+ ((rowParsed.addess >> 8) & 0xFF) + rowParsed.recordType;
	for (size_t i = 0; i < rowParsed.byteCount; i++) {
		sum += data[i];
	}
	sum = ~sum + 1;

	return sum == checksum;
}


static bool same(uint16_t ha, uint16_t la, uint8_t *data, size_t count){
	uint32_t address = ((uint32_t) ha) << 16 | la;
	uint8_t *flash = (uint8_t*)address;
	for(int i=0; i<count; i++){
		if(flash[i]!=data[i]){
			return false;
		}
	}
	return true;
}

static enum iHexCallbackRes ihexRowCheckSame(struct tHexRowParsed rowParsed,
		uint8_t *data, uint8_t checksum) {
	static uint16_t highAddress = 0;

	switch (rowParsed.recordType) {
	case IHEX_DATA:
		if(!same(highAddress, rowParsed.addess, data, rowParsed.byteCount)){
			return IHEX_CB_ERROR;
		}
		break;
	case IHEX_EOF:
		return IHEX_CB_EOF;
		break;
	case IHEX_EXT_LIN_ADDR:
		highAddress = ((uint16_t) data[0]) << 8 | data[1];
		break;
	case IHEX_START_LIN_ADDR:
		log_debug("Got Start Linear Address command, but it shall be ignored");
		break;
	case IHEX_EXT_SEG_ADDR:
	case IHEX_START_SEG_ADDR:
	default:
		log_error("Unhandled record type: %d", rowParsed.recordType);
	}
	return IHEX_CB_OK;
}

static bool writeData(uint16_t ha, uint16_t la, uint8_t *data, size_t count) {
	uint32_t address = ((uint32_t) ha) << 16 | la;
	return IntFlash_write(address, data, count);
}

static enum iHexCallbackRes ihexRowWriteMem(struct tHexRowParsed rowParsed,
		uint8_t *data, uint8_t checksum) {
	static uint16_t highAddress = 0;

	switch (rowParsed.recordType) {
	case IHEX_DATA:
		if(!writeData(highAddress, rowParsed.addess, data, rowParsed.byteCount)){
			return IHEX_CB_ERROR;
		}
		break;
	case IHEX_EOF:
		IntFlash_sync();
		return IHEX_CB_EOF;
		break;
	case IHEX_EXT_LIN_ADDR:
		highAddress = ((uint16_t) data[0]) << 8 | data[1];
		break;
	case IHEX_START_LIN_ADDR:
		log_debug("Got Start Linear Address command, but it shall be ignored");
		break;
	case IHEX_EXT_SEG_ADDR:
	case IHEX_START_SEG_ADDR:
	default:
		log_error("Unhandled record type: %d", rowParsed.recordType);
	}
	return IHEX_CB_OK;
}

static bool parseIntelHex(FIL *fp,
		enum iHexCallbackRes callback(struct tHexRowParsed, uint8_t*, uint8_t)) {
	struct tHexRowChars rowstart;
	struct tHexRowParsed rowParsed;

	char charbuf[512] = { 0 };
	uint8_t dataBuf[256] = { 0 };
	char checksumChar[2] = { 0 };
	uint8_t checksum;

	UINT br;
	for (;;) {
		if (!readUntilColon(fp)) {
			return true;
		}
		if (FR_OK != f_read(fp, &rowstart, sizeof(rowstart), &br)) {
			return false;
		}
		if (br != sizeof(rowstart)) {
			return false;
		}
		if (!hexRowCharToParsed(&rowstart, &rowParsed)) {
			return false;
		}
		if (FR_OK != f_read(fp, charbuf, rowParsed.byteCount * 2, &br)) {
			return false;
		}
		if (br != rowParsed.byteCount * 2) {
			return false;
		}
		for (int i = 0; i < rowParsed.byteCount; i++) {
			if (!hexPairToInt(charbuf + i * 2, dataBuf + i)) {
				return false;
			}
		}

		if (FR_OK != f_read(fp, checksumChar, 2, &br)) {
			return false;
		}
		if (br != 2) {
			return false;
		}
		if (!hexPairToInt(checksumChar, &checksum)) {
			return false;
		}

		enum iHexCallbackRes res;
		if (callback) {
			res = callback(rowParsed, dataBuf, checksum);
			if (res == IHEX_CB_ERROR)
				return false;
			if (res == IHEX_CB_EOF)
				return true;
		}
	}
	return true;
}

static bool firmwareAction(enum iHexCallbackRes callback(struct tHexRowParsed, uint8_t*, uint8_t)){
	FIL fp;
	if (FR_OK != f_open(&fp, APP_BIN_PATH, FA_READ)) {
		f_close(&fp);
		return false;
	}

	bool res = parseIntelHex(&fp, callback);

	f_close(&fp);
	return res;
}

static bool firmwareSame(){
	return firmwareAction(ihexRowCheckSame);
}

static bool firmwareOK() {
	return firmwareAction(iHexRowValidateChecksum);
}

static bool firmwareWrite(){
	if(HAL_OK!=IntFlash_unlock()){
		return false;
	}
	// TODO: IntFlash_erase_app();
	return firmwareAction(ihexRowWriteMem);
}

enum APL_RES AppLoader_check_firmware() {
	if(HAL_OK!=IntFlash_unlock()){
		return APPLOADER_ERROR;
	}
	if (!firmwareExists()) {
		return APPLOADER_NO_FIRMWARE;
	}
	if (!firmwareOK()) {
		return APPLOADER_CRC_MISMATCH;
	}
	return APPLOADER_OK;
}

bool AppLoader_update_firmware() {
	if(HAL_OK!=IntFlash_unlock()){
		return false;
	}
	if(firmwareSame()){
		return true;
	}
	firmwareWrite();
	return true;
}

bool AppLoader_verify_firmware() {
	if(HAL_OK!=IntFlash_unlock()){
		return false;
	}
	if(!firmwareSame()){
		log_error("Firmware validation failed");
		Error_Handler();
	}
	return true;
}

void AppLoader_load_application() {
	__disable_irq();
	const uint32_t app_start_address = 0x08040000;
	const uint32_t app_jump_address = *((__IO uint32_t*)(app_start_address+4));

	__IO uint32_t *estack = (__IO uint32_t*) app_start_address;
	uint32_t stack_pointer = *estack;

	void(*app)(void) = (void(*)(void)) app_jump_address;

	__set_MSP(stack_pointer);
	app();
}
