#ifndef INC_APPLOADER_H_
#define INC_APPLOADER_H_

enum APL_RES{
	APPLOADER_OK = 1,
	APPLOADER_NO_FIRMWARE,
	APPLOADER_NO_CRC_FILE,
	APPLOADER_CRC_MISMATCH,
	APPLOADER_FIRMWARE_SAME,
};

enum APL_RES AppLoader_check_firmware();

void AppLoader_update_firmware();
void AppLoader_verify_firmware();

void AppLoader_load_application();

#endif /* INC_APPLOADER_H_ */
