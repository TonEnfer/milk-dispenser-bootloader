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

typedef uint32_t tColor;

struct tTftFramebuffer{
	tColor *buffer;
	uint16_t width;
	uint16_t height;

	sFONT *font;
};


#define TFT_COLOR_WHITE ((tColor)0xFFFFFFFF)
#define TFT_COLOR_RED ((tColor)0xFF0000FF)
#define TFT_COLOR_BLACK ((tColor)0x000000FF)
#define TFT_COLOR_GRAY ((tColor)0x7F7F7FFF)


struct tTftFramebuffer TFT_init_framebuffer(LTDC_HandleTypeDef *hltdc);
void TFT_pixel(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, tColor color);
void TFT_fill(struct tTftFramebuffer buffer, tColor color);
void TFT_fill_rectangle(struct tTftFramebuffer buffer, uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1, tColor color);

void TFT_Char(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, char c, tColor color, tColor background);
void TFT_String(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, const char* str, tColor color, tColor background);

tColor color_rgb(uint8_t r, uint8_t g, uint8_t b);
tColor color_hex(uint32_t argb);

#endif /* INC_TFT_H_ */
