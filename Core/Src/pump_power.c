#include "pump_power.h"

extern TIM_HandleTypeDef htim1;

void pump_power_init(){
	HAL_TIM_PWM_Start(pump_power_config.timer, pump_power_config.channel);
}

void pump_pwoer_deinit(){
	HAL_TIM_PWM_Stop(pump_power_config.timer, pump_power_config.channel);
}
