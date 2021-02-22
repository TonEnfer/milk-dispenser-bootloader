/*
 * SST26VF064B quadspi flash chip driver
 */

#ifndef INC_SST26VF064B_H_
#define INC_SST26VF064B_H_

#include "stm32h7xx_hal.h"


HAL_StatusTypeDef SST26_init();

HAL_StatusTypeDef SST26_Read(uint32_t address, uint32_t size, uint8_t *buffer);

/**
 * This function is a dumb write. It does nothing smart about preserving any data or clearing it. It just performs a write operation on selected address.
 */
HAL_StatusTypeDef SST26_Write(uint32_t address, uint32_t size, uint8_t *buffer);
HAL_StatusTypeDef SST26_SectorWrite(uint32_t sectorNumber, uint32_t sectorCount, uint8_t *buffer);
HAL_StatusTypeDef SST26_EraseSector(uint32_t address);

HAL_StatusTypeDef SST26_ReadDeviceID(uint8_t *ID_REG);
HAL_StatusTypeDef SST26_Status(uint8_t *STATUS_REG);

// WARNING: This test overwrite flash memory with garbage. Dont use in production code!
void SST26_MemoryTest();

struct tSST26Config{
	QSPI_HandleTypeDef *hqspi;
	uint32_t timeout;
} SST26_config;

#endif /* INC_SST26VF064B_H_ */
