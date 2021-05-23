#ifndef TTY_HPP
#define TTY_HPP

#include "blit.hpp"

namespace E64 {

enum output_type {
	NOTHING,
	ASCII,
	BLITTER
};

class tty_t {
private:
	uint16_t cursor_position;
	
	uint8_t cursor_interval;
	uint8_t cursor_countdown;
	
	char     cursor_original_char;
	uint16_t cursor_original_color;
	uint16_t cursor_original_background_color;
	
	bool cursor_blinking;
	char *command_buffer;
	
	enum E64::output_type check_output(bool top_down, uint32_t *address);
	
public:
	tty_t(uint8_t flags_0, uint8_t flags_1, uint8_t size_in_tiles_log2, uint8_t _blit_no, blitter_ic *_blitter, uint16_t foreground_color, uint16_t background_color);
	~tty_t();
	
	uint8_t blit_no;
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
	
	inline int get_current_column()
	{
		return cursor_position % text_screen->get_columns();
	}
	
	inline int get_current_row()
	{
		return cursor_position / text_screen->get_columns();
	}

	inline int lines_remaining() { return text_screen->get_rows() - (cursor_position / text_screen->get_columns()) - 1; }

	char *enter_command();
	
	void timer_callback();
	
	void add_bottom_row();
	void add_top_row();
};

}

#endif
