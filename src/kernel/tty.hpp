#ifndef TTY_HPP
#define TTY_HPP

#include "blitter.hpp"

namespace E64 {

class tty_t {
private:
	uint8_t  columns;
	uint16_t rows;
	uint16_t tiles;
	
	uint16_t cursor_position;
	
	uint8_t cursor_interval;
	uint8_t cursor_countdown;
	char cursor_original_char;
	uint16_t cursor_original_color;
	uint16_t cursor_original_background_color;
	bool cursor_blink;	// current state
	char *command_buffer;
	
	uint16_t current_foreground_color;
	uint16_t current_background_color;
public:
	tty_t(uint8_t flags_0, uint8_t flags_1, uint8_t size_in_tiles_log2, int _blit_no, blitter_ic *_blitter, uint16_t foreground_color, uint16_t background_color);
	~tty_t();
	
	blit_t *text_screen;
	void clear();
	void putsymbol(char symbol);
	int putchar(int character);
	int puts(const char *text);
	int printf(const char *format, ...);
	
	void prompt();
	void activate_cursor();
	void deactivate_cursor();
	
	void cursor_left();
	void cursor_right();
	void cursor_up();
	void cursor_down();
	void backspace();
	
	char *enter_command();
	
	void timer_callback();
	
	void add_bottom_line();
};

}

#endif
