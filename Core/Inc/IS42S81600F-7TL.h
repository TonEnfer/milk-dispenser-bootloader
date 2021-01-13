/*
 * IS42S81600F-7TL.h
 *
 *  Created on: Jan 9, 2021
 *      Author: Mikhail
 */

#ifndef INC_IS42S81600F_7TL_H_
#define INC_IS42S81600F_7TL_H_

#include "fmc.h"

#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)
#define SDRAM_MODEREG_BURST_LENGTH_FULL_PAGE     ((uint16_t)0x0007)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



HAL_StatusTypeDef IS42S81600F_7TL_Init(SDRAM_HandleTypeDef *hsdram1);

#endif /* INC_IS42S81600F_7TL_H_ */
