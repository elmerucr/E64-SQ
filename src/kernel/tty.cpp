#include "tty.hpp"
#include "common.hpp"
#include <cstdarg>

#define COMMAND_BUFFER_SIZE 63+(3*64)

E64::tty_t::tty_t(uint8_t flags_0, uint8_t flags_1, uint8_t size_in_tiles_log2, int _blit_no, uint16_t foreground_color, uint16_t background_color)
{
	//text_screen = (surface_t *)machine.mmu->malloc(sizeof(surface_t));
	text_screen = &machine.blitter->blit[_blit_no];
	
	//blit_no = _blit_no;
	
	text_screen->flags_0 = flags_0;
	text_screen->flags_1 = flags_1;
	text_screen->size_in_tiles_log2 = size_in_tiles_log2;
	
	columns = (0b1 << (size_in_tiles_log2 & 0b111));
	rows = (0b1 << ((size_in_tiles_log2 & 0b1110000) >> 4));
	tiles = columns * rows;
	
	command_buffer = new char[COMMAND_BUFFER_SIZE];
	
	current_foreground_color = foreground_color;
	current_background_color = background_color;
}

E64::tty_t::~tty_t()
{
	delete command_buffer;
}

void E64::tty_t::clear()
{
	for (size_t i=0; i < tiles; i++) {
		text_screen->tile_data[i] = ' ';
		text_screen->tile_color_data[i] = current_foreground_color;
		text_screen->tile_background_color_data[i] = current_background_color;
	}
	
	cursor_position = 0;
//	cursor_start_of_command = 0;
//	cursor_end_of_command = 0;
	
	cursor_interval = 20; 	// 0.33s (if timer @ 60Hz)
	cursor_countdown = 0;
	cursor_blink = false;
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
	if (text) {
		while (*text) {
			putchar(*text);
			char_count++;
			text++;
		}
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
	
//	cursor_start_of_command -= columns;
//	cursor_end_of_command -= columns;
}

void E64::tty_t::prompt()
{
	printf("\nready.\n");
}

void E64::tty_t::activate_cursor()
{
	cursor_original_char = text_screen->tile_data[cursor_position];
	cursor_original_color = text_screen->tile_color_data[cursor_position];
	cursor_original_background_color = text_screen->tile_background_color_data[cursor_position];
	cursor_blink = true;
	cursor_countdown = 0;
}

void E64::tty_t::deactivate_cursor()
{
	cursor_blink = false;
	text_screen->tile_data[cursor_position] = cursor_original_char;
	text_screen->tile_color_data[cursor_position] = cursor_original_color;
	text_screen->tile_background_color_data[cursor_position] = cursor_original_background_color;
}

void E64::tty_t::timer_callback()
{
	if (cursor_blink == true) {
		if (cursor_countdown == 0) {
			text_screen->tile_data[cursor_position] ^= 0x80;
			cursor_countdown += cursor_interval;
		}
		cursor_countdown--;
	}
}

void E64::tty_t::cursor_left()
{
	uint16_t min_pos = 0;
//	if (current_edit_mode == SHELL)
//		min_pos = cursor_start_of_command;
	if (cursor_position > min_pos) cursor_position--;
}

void E64::tty_t::cursor_right()
{
	cursor_position++;
	if (cursor_position > tiles - 1) {
		add_bottom_line();
		cursor_position -= columns;
	}
}

void E64::tty_t::cursor_up()
{
	cursor_position -= columns;
	if (cursor_position >= tiles)
		cursor_position += columns;
}

void E64::tty_t::cursor_down()
{
	cursor_position += columns;
	if (cursor_position >= tiles) {
		add_bottom_line();
		cursor_position -= columns;
	}
}

void E64::tty_t::backspace()
{
	uint16_t pos = cursor_position;
	uint16_t min_pos = 0;

	if (pos > min_pos) {
		cursor_position--;
		while (pos % columns) {
			text_screen->tile_data[pos - 1] = text_screen->tile_data[pos];
			text_screen->tile_color_data[pos - 1] = text_screen->tile_color_data[pos];
			text_screen->tile_background_color_data[pos - 1] = text_screen->tile_background_color_data[pos];
			pos++;
		}
		text_screen->tile_data[pos - 1] = ' ';
		text_screen->tile_color_data[pos - 1] = current_foreground_color;
		text_screen->tile_background_color_data[pos - 1] = current_background_color;
	}
}

char *E64::tty_t::enter_command()
{
	uint16_t start_of_line = cursor_position - (cursor_position % columns);
	for (size_t i = 0; i < columns; i++) {
		command_buffer[i] = text_screen->tile_data[start_of_line + i];
	}
	size_t i = columns - 1;
	while (command_buffer[i] == ' ') i--;
	command_buffer[i + 1] = 0;
	
	return command_buffer;
}
