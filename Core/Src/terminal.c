#include "terminal.h"
#include "fonts.h"

#define FONT Font16

#define terminalWidth 72
#define terminalHeight 30

struct tTerminalBuffer {
	int text[terminalHeight][terminalWidth];
	tColor fg[terminalHeight][terminalWidth];
	tColor bg[terminalHeight][terminalWidth];
	uint16_t line;
	uint16_t symbol;
};

struct tTerminalConfig {
	struct tTerminalBuffer buffer;
	struct tTftFramebuffer *framebuffer;
	void (*newline_cb)();
	void (*before_newsymbol_cb)();
	tColor bg_color;
	tColor fg_color;
} terminal_config;

static void newline() {
	terminal_config.buffer.symbol = 0;
	terminal_config.buffer.line++;
	if (terminal_config.buffer.line == terminalHeight) {
		terminal_config.buffer.line = 0;
	}
	for (uint16_t i = 0; i < terminalWidth; i++) {
		terminal_config.buffer.text[terminal_config.buffer.line][i] = '\0';
		terminal_config.buffer.fg[terminal_config.buffer.line][i] =
		TFT_COLOR_WHITE;
		terminal_config.buffer.bg[terminal_config.buffer.line][i] =
		TFT_COLOR_BLACK;
	}
	if (terminal_config.newline_cb != NULL) {
		terminal_config.newline_cb();
	}
}

static void newsymbol() {
	if (terminal_config.before_newsymbol_cb != NULL) {
		terminal_config.before_newsymbol_cb();
	}
	terminal_config.buffer.symbol++;
	if (terminal_config.buffer.symbol == terminalWidth) {
		terminal_config.buffer.symbol = 0;
		newline();
	}

}

int __io_putchar(int ch) {
	if (ch == '\n') {
		newline();
	} else {
		uint16_t line = terminal_config.buffer.line;
		uint16_t symbol = terminal_config.buffer.symbol;
		terminal_config.buffer.text[line][symbol] = ch;
		terminal_config.buffer.fg[line][symbol] = terminal_config.fg_color;
		terminal_config.buffer.bg[line][symbol] = terminal_config.bg_color;
		newsymbol();
	}
	return ch;
}

int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		__io_putchar(*ptr++);
	}
	return len;
}

tColor terminal_color(tColor fg) {
	tColor backup = terminal_config.fg_color;
	terminal_config.fg_color = fg;
	return backup;
}

static uint16_t get_line_position(uint16_t buffer_line) {
	uint16_t first_line = (terminal_config.buffer.line + 1) % terminalHeight;
	if (buffer_line < first_line) {
		buffer_line += terminalHeight;
	}
	uint16_t line_on_screen = buffer_line - first_line;
	uint16_t line_y_on_screen = line_on_screen * FONT.Height;
	return line_y_on_screen;
}

static void repaint_char(uint16_t line, uint16_t symbol) {
	sFONT *backup = terminal_config.framebuffer->font;
	terminal_config.framebuffer->font = &FONT;

	uint16_t char_y = get_line_position(line);
	uint16_t char_x = symbol * FONT.Width;
	int ch = terminal_config.buffer.text[line][symbol];
	tColor bg = terminal_config.buffer.bg[line][symbol];
	tColor fg = terminal_config.buffer.fg[line][symbol];
	TFT_Char(*terminal_config.framebuffer, char_x, char_y, ch, fg, bg);

	terminal_config.framebuffer->font = backup;
}

static void repaint_active_char() {
	repaint_char(terminal_config.buffer.line, terminal_config.buffer.symbol);
}

static void repaint_line(uint16_t line) {
	sFONT *backup = terminal_config.framebuffer->font;
	terminal_config.framebuffer->font = &FONT;

	for (uint16_t symbol = 0; symbol < terminalWidth; symbol++) {
		repaint_char(line, symbol);
	}

	terminal_config.framebuffer->font = backup;
}


void repaint() {
	sFONT *backup = terminal_config.framebuffer->font;
	terminal_config.framebuffer->font = &FONT;
	for (uint16_t line = 0; line < terminalHeight; line++) {
		repaint_line(line);
	}
	terminal_config.framebuffer->font = backup;
}

void terminal_init(struct tTftFramebuffer *framebuffer) {
	terminal_config.newline_cb = repaint;
	terminal_config.before_newsymbol_cb = repaint_active_char;
	terminal_config.framebuffer = framebuffer;
	terminal_config.bg_color = TFT_COLOR_BLACK;
	terminal_config.fg_color = TFT_COLOR_WHITE;
//	setvbuf(stdout, NULL, _IONBF, 0);
	repaint();
}

