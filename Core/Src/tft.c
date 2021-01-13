#include "tft.h"

//volatile tColor _FrameBuffer_[400*240] = {0};

//volatile tColor *FrameBuffer = _FrameBuffer_;

volatile tColor *FrameBuffer = (tColor*)0xC0000000;

struct tTftFramebuffer TFT_init_framebuffer(LTDC_HandleTypeDef *hltdc) {
	HAL_LTDC_SetAddress(hltdc, (uint32_t) FrameBuffer, 0);
	return (struct tTftFramebuffer ) { (tColor*) FrameBuffer, 792, 472, &Font12 } ;
}

void TFT_pixel(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, uint32_t color) {
	buffer.buffer[y * buffer.width + x] = color_hex(color);
}

void TFT_fill(struct tTftFramebuffer buffer, uint32_t color) {
	tColor innerColor = color_hex(color);
	uint32_t bufferLength = buffer.width*buffer.height;
	for (int i = 0; i < bufferLength; i++) {
		buffer.buffer[i] = innerColor;
	}
}

void TFT_fill_rectangle(struct tTftFramebuffer buffer, uint16_t x0,
		uint16_t x1, uint16_t y0, uint16_t y1, uint32_t color) {
	tColor innerColor = color_hex(color);
	for (uint16_t x = x0; x < x1; x++) {
		for (uint16_t y = y0; y < y1; y++) {
			buffer.buffer[y * buffer.width + x] = innerColor;
		}
	}
}

void TFT_Char(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, char c, uint32_t color, uint32_t background) {
	uint16_t fontWidth = buffer.font->Width;
	uint16_t fontHeight = buffer.font->Height;
	const uint8_t *fontTable = buffer.font->table;

	const uint8_t *fontChar = fontTable + (c - ' ') * fontHeight * ((fontWidth + 7) / 8);

	uint16_t offset = 8 * ((fontWidth + 7) / 8) - fontWidth;
	uint64_t line;
	for (uint16_t i = 0; i < fontHeight; i++) {
		const uint8_t *pchar = fontChar + (fontWidth + 7) / 8 * i;
		switch ((fontWidth + 7) / 8) {
		case 1:
			line = pchar[0];
			break;
		case 2:
			line = (pchar[0] << 8) | pchar[1];
			break;
		case 3:
		default:
			line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
			break;
		}
		for (uint16_t j = 0; j < fontWidth; j++) {
			if (line & (1 << (fontWidth - j + offset - 1))) {
				TFT_pixel(buffer, (x + j), y, color);
			} else {
				TFT_pixel(buffer, (x + j), y, background);
			}
		}
		y++;
	}
}


void TFT_String(struct tTftFramebuffer buffer, uint16_t x, uint16_t y, const char* str, uint32_t color, uint32_t background){
	uint16_t currentX = x;
	uint16_t currentY = y;
	for(const char* current_char = str; *current_char; current_char++){
		if (*current_char >= 0x20){
			TFT_Char(buffer, currentX, currentY, *current_char, color, background);
		}
		currentX+=buffer.font->Width;
		if(currentX+buffer.font->Width > buffer.width || *current_char == '\n'){
			currentX=0;
			currentY+=buffer.font->Height;
		}
	}
}


tColor color_rgb(uint8_t r, uint8_t g, uint8_t b) {
	return ((tColor) ((r >> 3) & 0x1F)) << (6 + 5)
			| ((tColor) ((g >> 2) & 0x3F)) << (5)
			| ((tColor) ((b >> 3) & 0x1F));
}

tColor color_hex(uint32_t argb) {
	return argb;
//	return color_rgb((argb & 0xFF0000) >> 16, (argb & 0xFF00) >> 8,
//			argb & 0xFF);
}
