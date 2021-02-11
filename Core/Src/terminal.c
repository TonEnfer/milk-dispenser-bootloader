#include "terminal.h"

static void increment_buffer() {
	terminal_config.buffer.finish++;
	if (terminal_config.buffer.finish == ringBufferSize) {
		terminal_config.buffer.finish = 0;
	}
	if (terminal_config.buffer.finish == terminal_config.buffer.start) {
		terminal_config.buffer.start++;
	}
	if (terminal_config.buffer.start == ringBufferSize) {
		terminal_config.buffer.start = 0;
	}
}

static volatile uint8_t will_repaint = 0;

int __io_putchar(int ch) {
	will_repaint ++;
	terminal_config.buffer.buf[terminal_config.buffer.finish] = ch;
	increment_buffer();
	will_repaint --;
	if(will_repaint == 0){
		repaint();
	}
	return ch;
}

int _write(int file, char *ptr, int len) {
	will_repaint ++;
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		__io_putchar(*ptr++);
	}
	will_repaint --;
	if(will_repaint == 0){
		repaint();
	}
	return len;
}

void init() {
	repaint();
}

void repaint(){
	uint32_t start = terminal_config.buffer.start;
	uint32_t finish = terminal_config.buffer.finish;

	uint32_t diff = finish-start;
	if(finish < start){

	}

	for (uint32_t i=start; i<ringBufferSize; i++){

	}
}

