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

void terminal_init(struct tTftFramebuffer *framebuffer);
int fputc(int ch, FILE *f);
int fputs(const char*__restrict, FILE*__restrict);

tColor terminal_color(tColor fb);
#endif /* INC_TERMINAL_H_ */
