/*
 * terminal.h
 *
 *  Created on: Feb 10, 2021
 *      Author: Mikhail
 */

#ifndef INC_TERMINAL_H_
#define INC_TERMINAL_H_
#include <stdlib.h>
#include <stdio.h>

#include "tft.h"

#define ringBufferSize 10080

struct tRingBuffer {
	int buf[ringBufferSize];
	tColor cBuf[ringBufferSize];
	uint32_t start;
	uint32_t finish;
};

struct tTerminalConfig {
	uint16_t tTftFramebuffer;
	struct tRingBuffer buffer;
	tColor bg_color;
	tColor fg_color;
} terminal_config;

void init();
int fputc(int ch, FILE *f);
int	fputs (const char *__restrict, FILE *__restrict);

void repaint();
#endif /* INC_TERMINAL_H_ */
