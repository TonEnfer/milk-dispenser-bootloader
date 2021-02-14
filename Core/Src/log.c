#include "log.h"
#include "terminal.h"
#include <stdarg.h>

void stub(){
	asm("nop");
}

void _log(eLogLevel level, const char* format, ...){
	tColor backup;
	switch(level){
	case LL_DEBUG:
	case LL_INFO:
		backup = terminal_color(TFT_COLOR_WHITE);
		break;
	case LL_WARN:
		backup = terminal_color(TFT_COLOR_YELLOW);
		break;
	case LL_ERROR:
		backup = terminal_color(TFT_COLOR_RED);
		break;
	default:
		backup = terminal_color(TFT_COLOR_BLACK);
		break;
	}
	printf("[%8ld] ", HAL_GetTick());

    va_list args;
    va_start(args, format);

	vprintf(format, args);

    va_end(args);

    printf("\n");
    terminal_color(backup);
}
