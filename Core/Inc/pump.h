/*
 * pump.h
 *
 *  Created on: 1 февр. 2021 г.
 *      Author: Mikhail
 */

#ifndef INC_PUMP_H_
#define INC_PUMP_H_

#include "stm32h7xx_hal.h"

void pump_init();

void pump_on();

void pump_off();

struct tPumpConfig {
	GPIO_TypeDef* port;
	uint16_t pin;
} pump_config;

#endif /* INC_PUMP_H_ */
