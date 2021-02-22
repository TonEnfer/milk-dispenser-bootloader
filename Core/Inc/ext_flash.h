/*
 * ext_flash.h
 *
 *  Created on: Feb 21, 2021
 *      Author: Mikhail
 */

#ifndef INC_EXT_FLASH_H_
#define INC_EXT_FLASH_H_

#include "stm32h7xx_hal.h"

HAL_StatusTypeDef Flash2Write(uint32_t address, uint8_t *buffer, uint32_t size);

HAL_StatusTypeDef Flash2Read(uint32_t address, uint8_t *buffer, uint32_t size);

#endif /* INC_EXT_FLASH_H_ */
