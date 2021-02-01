#include "pump.h"

void pump_init(){
	pump_off();
}


void pump_on(){
	HAL_GPIO_WritePin(pump_config.port, pump_config.pin, GPIO_PIN_SET);
}

void pump_off(){
	HAL_GPIO_WritePin(pump_config.port, pump_config.pin, GPIO_PIN_RESET);
}

