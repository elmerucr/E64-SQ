#include "tty.hpp"
#include "common.hpp"
#include <cstdarg>

E64::tty_t::tty_t(uint8_t size_in_tiles_log2, uint16_t *pixeldata, uint16_t foreground_color, uint16_t background_color)
{
	text_screen = (surface_t *)machine.mmu->malloc(sizeof(surface_t));
	
	text_screen->flags_0 = 0b00001000;
	text_screen->flags_1 = 0b00000000;
	
	text_screen->size_in_tiles_log2 = size_in_tiles_log2;
	columns = (0b1 << (size_in_tiles_log2 & 0b111));
	rows = (0b1 << ((size_in_tiles_log2 & 0b1110000) >> 4));
	tiles = columns * rows;
	
	current_foreground_color = foreground_color;
	current_background_color = background_color;
	
	text_screen->pixel_data = pixeldata;
	text_screen->tile_data = (uint8_t *)machine.mmu->malloc(tiles * sizeof(uint8_t));
	text_screen->tile_color_data = (uint16_t *)machine.mmu->malloc(tiles * sizeof(uint16_t));
	text_screen->tile_background_color_data = (uint16_t *)machine.mmu->malloc(tiles * sizeof(uint16_t));
	
	clear();
}

E64::tty_t::~tty_t()
{
	machine.mmu->free(text_screen->tile_background_color_data);
	machine.mmu->free(text_screen->tile_color_data);
	machine.mmu->free(text_screen->tile_data);
	machine.mmu->free(text_screen);
}

void E64::tty_t::clear()
{
	for (size_t i=0; i < tiles; i++) {
		text_screen->tile_data[i] = ' ';
		text_screen->tile_color_data[i] = current_foreground_color;
		text_screen->tile_background_color_data[i] = current_background_color;
	}
	
	cursor_position = 0;
//	tty_current->cursor_position = 0;
//	tty_current->cursor_start_of_command = 0;
//	tty_current->cursor_end_of_command = 0;
}

void E64::tty_t::putsymbol(char symbol)
{
	text_screen->tile_data[cursor_position] = symbol;
	text_screen->tile_color_data[cursor_position] = current_foreground_color;
	text_screen->tile_background_color_data[cursor_position] = current_background_color;
	cursor_position++;
	if (cursor_position >= tiles) {
		add_bottom_line();
		cursor_position -= columns;
	}
}

int E64::tty_t::putchar(int character)
{
	uint8_t result = (uint8_t)character;
	switch (result) {
		case '\r':
			cursor_position -= cursor_position % columns;
			break;
		case '\n':
			cursor_position -= cursor_position % columns;
			if ((cursor_position / columns) == (rows - 1)) {
				    add_bottom_line();
			} else {
				cursor_position += columns;
			}
			break;
		case '\t':
			while ((cursor_position % columns) & 0b11) {
				putsymbol(' ');
			}
			break;
		default:
			putsymbol(result);
			break;
	}
	
	return result;
}

int E64::tty_t::puts(const char *text)
{
	int char_count = 0;
	while (*text) {
		putchar(*text);
		char_count++;
		text++;
	}
	return char_count;
}

int E64::tty_t::printf(const char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	int number = vsnprintf(buffer, 1024, format, args);
	va_end(args);
	puts(buffer);
	return number;
}

void E64::tty_t::add_bottom_line()
{
	uint16_t no_of_tiles_to_move = tiles - columns;

	for (size_t i=0; i < no_of_tiles_to_move; i++) {
		text_screen->tile_data[i] = text_screen->tile_data[i + columns];
		text_screen->tile_color_data[i] = text_screen->tile_color_data[i + columns];
		text_screen->tile_background_color_data[i] = text_screen->tile_background_color_data[i + columns];
	}
	for (size_t i=no_of_tiles_to_move; i < tiles; i++) {
		text_screen->tile_data[i] = ' ';
		text_screen->tile_color_data[i] = current_foreground_color;
		text_screen->tile_background_color_data[i] = current_background_color;
	}
	
	//tty_current->cursor_start_of_command -= tty_current->columns;
	//tty_current->cursor_end_of_command -= tty_current->columns;
}
