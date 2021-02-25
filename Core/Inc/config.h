#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include <stdint.h>

struct tBootloaderConfig {
	uint64_t application_runs;
	uint64_t application_crc;
	uint64_t wait_for_flash_activation_confirmation_ms;
	uint64_t activate_flash_drive_timeout_ms;
	uint64_t activate_main_program_timeout_ms;
};


struct tBootloaderConfig* BtldrConfig_Get();

uint64_t BtldrConfig_incrementRuns();


#endif /* INC_CONFIG_H_ */
