/*
 * IS42S81600F-7TL.c
 *
 *  Created on: Jan 9, 2021
 *      Author: Mikhail
 */

#include "IS42S81600F-7TL.h"


HAL_StatusTypeDef IS42S81600F_7TL_Init(SDRAM_HandleTypeDef *hsdram1){
	FMC_SDRAM_CommandTypeDef command;

	HAL_Delay(1);

	command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 2;
	command.ModeRegisterDefinition = 0;
	HAL_StatusTypeDef sdram_cmd_status = HAL_SDRAM_SendCommand(hsdram1, &command, SDRAM_TIMEOUT);
	HAL_Delay(1);

	command.CommandMode = FMC_SDRAM_CMD_PALL;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 2;
	command.ModeRegisterDefinition = 0;
	sdram_cmd_status = HAL_SDRAM_SendCommand(hsdram1, &command, SDRAM_TIMEOUT);
	HAL_Delay(1);

	command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 2;
	command.ModeRegisterDefinition = 0;
	sdram_cmd_status = HAL_SDRAM_SendCommand(hsdram1, &command, SDRAM_TIMEOUT);
	HAL_Delay(1);

	command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 2;
	command.ModeRegisterDefinition = SDRAM_MODEREG_BURST_LENGTH_1 |
							SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL  |
							SDRAM_MODEREG_CAS_LATENCY_2           |
							SDRAM_MODEREG_OPERATING_MODE_STANDARD |
							SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
	sdram_cmd_status = HAL_SDRAM_SendCommand(hsdram1, &command, SDRAM_TIMEOUT);
	HAL_Delay(1);

	hsdram1->Instance->SDRTR = ((uint32_t)1000)<<1;

	return sdram_cmd_status;
}

