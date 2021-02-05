/*
 * SST26VF064B quadspi flash chip driver
 */

#ifndef INC_SST26VF064B_H_
#define INC_SST26VF064B_H_

#include "stm32h7xx_hal.h"


HAL_StatusTypeDef SST26_init();

HAL_StatusTypeDef SST26_Read(uint32_t address, uint32_t size, uint8_t *buffer);

HAL_StatusTypeDef SST26_ReadDeviceID(uint8_t *ID_REG);

struct tSST26Config{
	QSPI_HandleTypeDef *hqspi;
	uint32_t timeout;
} SST26_config;

#endif /* INC_SST26VF064B_H_ */
