/*
 * display.h
 *
 *  Created on: Jan 6, 2021
 *      Author: Mikhail
 */

#ifndef INC_TFT_H_
#define INC_TFT_H_

#include "stm32h7xx_hal.h"

#include "fonts.h"
#include "tim.h"

typedef uint16_t tColor;

struct tTftFramebuffer{
	tColor *buffer;
	uint16_t width;
	uint16_t height;

	sFONT *font;
};

#define c565(R,G,B) ((tColor)( ((R&0Xf8)<<8)|((G&0xFC)<<3)|((B&0xF8)>>3) ))

#define TFT_COLOR_WHITE c565(255,255,255)
#define TFT_COLOR_YELLOW c565(255,255,0)
#define TFT_COLOR_RED c565(255,0,0)
#define TFT_COLOR_BLACK c565(0,0,0)
#define TFT_COLOR_GRAY c565(127,127,127)


struct tTftFramebuffer TFT_init_framebuffer(LTDC_HandleTypeDef *hltdc);
void TFT_pixel(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, tColor color);
void TFT_fill(struct tTftFramebuffer buffer, tColor color);
void TFT_fill_rectangle(struct tTftFramebuffer buffer, uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1, tColor color);

void TFT_Char(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, int c, tColor color, tColor background);
void TFT_String(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, const char* str, tColor color, tColor background);

tColor color_rgb(uint8_t r, uint8_t g, uint8_t b);
tColor color_hex(uint32_t argb);

void TFT_Set_brightness(uint16_t brightness);

#endif /* INC_TFT_H_ */
