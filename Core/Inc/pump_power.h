/*
 * pump_power.h
 *
 *  Created on: Feb 1, 2021
 *      Author: Mikhail
 */

#ifndef INC_PUMP_POWER_H_
#define INC_PUMP_POWER_H_
#include "stm32h7xx_hal.h"
#include "tim.h"

void pump_power_init();
void pump_pwoer_deinit();

struct tPumpPowerConfig{
	TIM_HandleTypeDef* timer;
	uint32_t channel;
} pump_power_config;

#endif /* INC_PUMP_POWER_H_ */
