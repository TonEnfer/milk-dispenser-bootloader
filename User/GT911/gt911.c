#include "GT911.h"

#include <stdio.h>
#include "i2c.h"
#define PRIMITIVE_TIMEOUT 1000

volatile struct GT911 gt911;
volatile struct GT911 shadow;


static void GT911_Reset()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_Delay(100);
	// Hold INT pin low to select 0xBA/0xBB address
	// TODO: implement selecting any address needed
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_Delay(100);

	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_Delay(100);
	HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}


static HAL_StatusTypeDef GT911_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len){
	uint8_t buf_with_reg[len+2];
	buf_with_reg[0] = (reg >> 8)&0xFF;
	buf_with_reg[1] = reg&0xFF;
	for(uint8_t i=0; i<len; i++){
		buf_with_reg[i+2] = buf[i];
	}
	return HAL_I2C_Master_Transmit(&hi2c1, GT911_CMD_WR_ADDR, buf_with_reg, len+2, PRIMITIVE_TIMEOUT);
}

HAL_StatusTypeDef GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len){
	HAL_StatusTypeDef status = GT911_WR_Reg(reg, buf, 0);
	if(status!=HAL_OK){
		return status;
	}
	return HAL_I2C_Master_Receive(&hi2c1, GT911_CMD_WR_ADDR, buf, len, PRIMITIVE_TIMEOUT);
}

HAL_StatusTypeDef GT911_Read_ID(uint8_t* buf, size_t len){
	if(GT911_RD_Reg(GT911_PRODUCT_ID_REG, buf, len)!=HAL_OK){
		return HAL_ERROR;
	}
	uint8_t config_reg;
	if(GT911_RD_Reg(GT911_CONFIG_REG, &config_reg, 1)!=HAL_OK){
			return HAL_ERROR;
		}
	__NOP();
	return HAL_OK;
}


static HAL_StatusTypeDef GT911_Ready_Reg(uint8_t *ready_reg){
	return GT911_RD_Reg(GT911_READY_REG, ready_reg, 1);
}

static HAL_StatusTypeDef GT911_Ready_Reg_Clear(){
	uint8_t buf = 0;
	return GT911_WR_Reg(GT911_READY_REG, &buf, 1);
}

static HAL_StatusTypeDef GT911_Scan_Impl(uint32_t timeout){
	HAL_StatusTypeDef status;
	uint32_t time_start = HAL_GetTick();

	uint8_t ready_reg = 0x7F;
	while(1){
		if(HAL_GetTick() - time_start > timeout){
			return HAL_TIMEOUT;
		}
		status = GT911_Ready_Reg(&ready_reg);
		if(status!=HAL_OK){
			return status;
		}
		if (ready_reg & GT911_READY_REG_BUFFER_STATUS){
			break;
		}
		HAL_Delay(5);
	}

	const uint8_t touch_points = ready_reg & GT911_READY_REG_NUMBER_OF_TOUCH_POINTS;
	if(touch_points > GT911_MAX_TOUCH){
		return HAL_ERROR;
	}

	shadow.TouchpointFlag = ready_reg;
	shadow.TouchCount = touch_points;

	if(touch_points == 0){
		status = GT911_Ready_Reg_Clear();
		return status;
	}

	const uint16_t coordinates_buffer_size = touch_points * 8;
	uint8_t coordinates_buffer[coordinates_buffer_size];
	for(size_t i=0; i<coordinates_buffer_size; i++){
		coordinates_buffer[i] = 0;
	}

	if(GT911_RD_Reg(GT911_COORDINATES_REG_START, coordinates_buffer, coordinates_buffer_size-2)!=HAL_OK){
		return HAL_ERROR;
	}

	if(GT911_Ready_Reg_Clear()!=HAL_OK){
		return HAL_ERROR;
	}

	GT911_TouchInfo* touches = (GT911_TouchInfo*)coordinates_buffer;

	for(size_t i=0; i<touch_points; i++){
		shadow.Touches[i] = touches[i];
	}

	return HAL_OK;

}

HAL_StatusTypeDef GT911_Scan(uint32_t timeout){
	shadow.Touch = 1;
	HAL_StatusTypeDef status = GT911_Scan_Impl(timeout);
	shadow.status = status;
	return status;
}


void GT911_CopyShadow(){
	gt911 = shadow;
	shadow.Touch = 0;
}


HAL_StatusTypeDef GT911_Init()
{
	printf("GT911_Init\r\n");
	GT911_Reset();
	return HAL_OK;
}

