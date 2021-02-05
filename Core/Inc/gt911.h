/*
 * gt911.h
 *
 *  Created on: Jan 13, 2021
 *      Author: Mikhail
 */

#ifndef GT911_GT911_H_
#define GT911_GT911_H_

#include <stdint.h>
#include <stm32h7xx.h>


#define GT911_MAX_WIDTH		1024    	//Touchscreen pad max width
#define GT911_MAX_HEIGHT		600			//Touchscreen pad max height

#define GT911_CMD_WR_ADDR		0XBA 					//Write data command
#define GT911_CMD_RD_ADDR   	0XBB 				//Read data command
//#define GT911_CMD_WR_ADDR		(0X14 << 1)					//Write data command
//#define GT911_CMD_RD_ADDR   	GT911_CMD_WR_ADDR+1 				//Read data command

//The maximum number of points supported by the capacitive touch screen
#define GT911_MAX_TOUCH    5

#define GT911_COMMAND_REG   				0x8040	/* Real-time command */
#define GT911_CONFIG_REG						0x8047	/* Configuration parameter register */
#define GT911_PRODUCT_ID_REG 				0x8140 	/* Product ID */
#define GT911_FIRMWARE_VERSION_REG  0x8144  /* Firmware version number */
#define GT911_READY_REG 					0x814E	/* Coordinates ready register */
#define GT911_READY_REG_BUFFER_STATUS 0x80 // buffer is ready = 1
#define GT911_READY_REG_LARGE_DETECT 0x40 // large-area touch = 1
#define GT911_READY_REG_TOUCH_KEY 0x10 // have touch key = 1, released = 0
#define GT911_READY_REG_NUMBER_OF_TOUCH_POINTS 0x0F // number of touch points detected

#define GT911_COORDINATES_REG_START 0x8150

#pragma pack(1)
typedef __PACKED_STRUCT  {
	uint16_t point_x;
	uint16_t point_y;
	uint16_t point_size;
	uint16_t __reserved__;
} GT911_TouchInfo;
#pragma pack()


struct GT911_Init{

};

struct GT911
{
	GT911_TouchInfo Touches[GT911_MAX_TOUCH];

	volatile uint8_t Touch;
	HAL_StatusTypeDef status;

	uint8_t TouchpointFlag;
	uint8_t TouchCount;

};

extern volatile struct GT911 gt911;

HAL_StatusTypeDef GT911_Init();
HAL_StatusTypeDef GT911_Scan(uint32_t timeout);

void GT911_CopyShadow();

HAL_StatusTypeDef GT911_Read_ID(uint8_t* buf, size_t len);

HAL_StatusTypeDef GT911_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len);

#endif /* GT911_GT911_H_ */
