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
	
	uint16_t current_foreground_color;
	uint16_t current_background_color;
public:
	tty_t(uint8_t size_in_tiles_log2, uint16_t *pixeldata, uint16_t foreground_color, uint16_t background_color);
	~tty_t();
	
	surface_t *text_screen;
	void clear();
	void putsymbol(char symbol);
	int putchar(int character);
	int puts(const char *text);
	int printf(const char *format, ...);
	
	void add_bottom_line();
};

}

#endif
