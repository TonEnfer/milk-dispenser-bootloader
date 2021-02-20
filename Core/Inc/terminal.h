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

int __io_putchar(int ch);
int _write(int file, char *ptr, int len);

tColor terminal_color(tColor fb);
#endif /* INC_TERMINAL_H_ */
