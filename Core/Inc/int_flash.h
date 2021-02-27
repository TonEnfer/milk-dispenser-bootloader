/*
 * int_flash.h
 *
 *  Created on: Feb 25, 2021
 *      Author: Mikhail
 */

#ifndef INC_INT_FLASH_H_
#define INC_INT_FLASH_H_
#include <stm32h7xx_hal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

HAL_StatusTypeDef IntFlash_unlock();
bool IntFlash_write(uint32_t address, uint8_t *buff, size_t bw);
bool IntFlash_sync();

#endif /* INC_INT_FLASH_H_ */
