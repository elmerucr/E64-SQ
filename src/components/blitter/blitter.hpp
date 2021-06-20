//  blitter.hpp
//  E64
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

/*
 *
 * I/O addresses blit_ic
 *
 * 0x00: control register
 * 0x01: blit_no
 * 0x02: hor border size
 * 0x03: -
 * 0x04: x_pos blit, low byte
 * 0x05: x_pos blit, high byte
 * 0x06: y_pos blit, low byte
 * 0x07: y_pos blit high byte
 * 0x08: clearcolor, low byte
 * 0x09: clearcolor, high byte
 * 0x0a: hor border color, low byte
 * 0x0b: hor border color, high byte
 * 0x0c:
 * 0x0d:
 * 0x0e: memory access page (low byte)
 * 0x0f: memory access page (high byte)
 *
 * 0x10: console related variables (based on blit_no @ 0x01)
 * =========================================================
 * 0x10/0x11: no of tiles
 * 0x12/0x13: cursor position
 *
 */

/*
 * Blitter is able to copy data very fast from video memory location to
 * backbuffer (framebuffer). Copy operations run independently and can be added
 * to a FIFO linked list (blitter internally).
 */

#ifndef BLIT_HPP
#define BLIT_HPP

#include <cstdint>

#define COMMAND_BUFFER_SIZE 63+(3*64)

namespace E64
{

enum output_type {
	NOTHING,
	ASCII,
	BLITTER
};

/*
 * The next structure is a surface blit. It is also used for terminal type
 * operations.
 */

class blit_t {
private:
	/*  Size of blit in tiles log2, 8 bit unsigned number.
	 *
	 *  7 6 5 4 3 2 1 0
	 *    | | |   | | |
	 *    | | |   +-+-+-- Low nibble  (just 3 of 4 bits)
	 *    | | |
	 *    +-+-+---------- High nibble (just 3 of 4 bits)
	 *
	 *  Low nibble codes for width (in tiles log2) of the blit.
	 *  High nibble codes for height.
	 *
	 *  Bits 3 and 7: Reserved.
	 *
	 *  The 3 least significant bits of each nibble indicate a number of
	 *  0 - 7 (n). Finally, a bit shift occurs: 0b00000001 << n (2^n)
	 *  Resulting in the final width/height in 'tiles' (8 pixels per tile)
	 *  { 1, 2, 4, 8, 16, 32, 64, 128 }
	 */
	uint8_t size_in_tiles_log2;
	
	uint8_t  columns;
	uint16_t rows;
public:
	uint16_t tiles;

	uint16_t cursor_position;
	uint8_t  cursor_interval;
	uint8_t  cursor_countdown;
	char     cursor_original_char;
	uint16_t cursor_original_color;
	uint16_t cursor_original_background_color;
	bool     cursor_blinking;
	char     command_buffer[COMMAND_BUFFER_SIZE];
	enum     E64::output_type check_output(bool top_down, uint32_t *address);

	/*  Flags 0
	 *
	 *  7 6 5 4 3 2 1 0
	 *  |       | | | |
	 *  |       | | | +-- Tile Mode (0) / Bitmap Mode (1)
	 *  |       | | +---- Background (0 = off, 1 = on)
	 *  |       | +------ Simple Color (0) / Multi Color (1)
	 *  |       +-------- Color per tile (0 = off, 1 = on)
	 *  +---------------- Pixel data video ram (0) / Or cbm font (1)
	 *
	 *  bits 4-7: Reserved
	 */
	uint8_t     flags_0;

	/*  Flags 1 - Stretching & Flips
	 *
	 *  7 6 5 4 3 2 1 0
	 *      | |   |   |
	 *      | |   |   +-- Horizontal stretching (0 = off, 1 = on)
	 *      | |   +------ Vertical stretching (0 = off, 1 = on)
	 *      | +---------- Horizontal flip (0 = off, 1 = on)
	 *      +------------ Vertical flip (0 = off, 1 = on)
	 *
	 *  bits 1, 3, 6 and 7: Reserved
	 */
	uint8_t     flags_1;
	
	/*
	 * Reserved byte for future purposes related to e.g. wrapping
	 */
	uint8_t currently_unused;
    
	/*
	 * Contains the foreground color (for both single color AND current color)
	 */
	uint16_t foreground_color;
    
	/*
	 * Contains the background color (for both single color AND current color)
	 */
	uint16_t background_color;
    
	/*
	 * Pointer to pixels (can be tile pixels or bitmap pixels)
	 */
	uint16_t *pixel_data;
    
	/*
	 * 32 bit pointer to start of tiles
	 */
	uint8_t *tile_data;
    
	/*
	 * 32 bit pointer to start of tile color
	 */
	uint16_t *tile_color_data;
    
	/*
	 * 32 bit pointer to start of tile background color
	 */
	uint16_t *tile_background_color_data;
	
	inline void set_size_in_tiles_log2(uint8_t size)
	{
		size_in_tiles_log2 = size & 0b01110111;
		columns = (0b1 << (size_in_tiles_log2 & 0b00000111));
		rows = (0b1 << ((size_in_tiles_log2 & 0b01110000) >> 4));
		tiles = columns * rows;
	}
	
	inline uint8_t get_size_in_tiles_log2()
	{
		return size_in_tiles_log2;
	}
	
	inline uint8_t get_columns() { return columns; }
	inline uint16_t get_rows() { return rows; }
	inline uint16_t get_tiles() { return tiles; }
	
	// terminal interface NEEDS WORK
	void terminal_init(uint8_t flags_0, uint8_t flags_1,
			   uint8_t size_in_tiles_log2, uint16_t foreground_color,
			   uint16_t background_color);
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
	void add_bottom_row();
	void add_top_row();
	inline int lines_remaining() { return rows - (cursor_position / columns) - 1; }
	inline int get_current_column()
	{
		return cursor_position % columns;
	}
	inline int get_current_row()
	{
		return cursor_position / columns;
	}
	void process_cursor_state();
	char *enter_command();
};

enum operation_type {
	CLEAR,
	BORDER,
	BLIT
};

struct operation {
	enum operation_type type;
	blit_t *blit_pointer;
	int16_t x_pos;
	int16_t y_pos;
};

enum blitter_state_t {
	IDLE,
	CLEARING,
	DRAW_BORDER,
	BLITTING
};

class blitter_ic {
private:
	uint8_t	registers[32];
	uint8_t *blit_memory;
	uint16_t *cbm_font;	// pointer to unpacked font
	
	enum blitter_state_t blitter_state;
	
	// framebuffer pointers
	uint16_t *fb0;
	uint16_t *fb1;

	/*
	 * Circular buffer containing operations. If more than 65536 operations
	 * would be written (unlikely) and unfinished, something will be
	 * overwritten.
	 */
	struct operation operations[65536];
	uint16_t head;
	uint16_t tail;
    
	/* Finite state machine */
	bool        bitmap_mode;
	bool        background;
	bool        multicolor_mode;
	bool        color_per_tile;
	bool        use_cbm_font;
	bool        hor_flip;
	bool        ver_flip;
	uint16_t    double_width;
	uint16_t    double_height;
    
	uint16_t width_in_tiles_log2;   // width of the blit source in multiples of 8 pixels and log2
	uint16_t width_log2;            // width of the blit source in pixels and log2
	uint16_t width_on_screen_log2;  // width of the final bit on screen in pixels (might be doubled) and log2
	uint16_t width;                 // width of the blit source in pixels
	uint16_t width_on_screen;       // width of the final bit on screen in pixels (might be doubled)
	
	uint16_t height_in_tiles_log2;  // height of the blit source in multiples of 8 pixels and log2
	uint16_t height_log2;           // height of the blit source in pixels and log2
	uint16_t height_on_screen_log2; // height of the final bit on screen in pixels (might be doubled) and log2
	uint16_t height;                // height of the blit source in pixels
	uint16_t height_on_screen;      // height of the final bit on screen in pixels (might be doubled)
	
	uint32_t total_no_of_pix;       // total number of pixels to blit onto framebuffer for this blit
	uint32_t pixel_no;              // current pixel of the total that is being processed
	uint32_t normalized_pixel_no;   // normalized to the dimensions of source
    
	// specific for clearing framebuffer
	uint16_t clear_color;
	
	// specific for border
	uint16_t border_color;
	uint8_t  border_size;

	int16_t x;
	int16_t y;
	
	uint16_t scrn_x;            // final screen x for the current pixel
	uint16_t scrn_y;            // final screen y for the current pixel
	
	uint16_t x_in_blit;
	uint16_t y_in_blit;
	
	uint16_t tile_x;
	uint16_t tile_y;
	
	
	uint16_t tile_number;
	uint8_t  tile_index;
	uint16_t current_background_color;
	uint8_t  pixel_in_tile;
	
	uint16_t source_color;
	
	uint16_t width_mask;
	uint16_t width_on_screen_mask;
    
	uint16_t foreground_color;
	uint16_t background_color;
    
	uint16_t *pixel_data;
	uint8_t *tile_data;
	uint16_t *tile_color_data;
	uint16_t *tile_background_color_data;
	uint32_t user_data;
	
	inline void check_new_operation();
public:
	blitter_ic();
	~blitter_ic();
	
	/* io access to blitter_ic (mapped to a specific page in memory) */
	uint8_t	io_read_8(uint8_t address);
	void	io_write_8(uint8_t address, uint8_t byte);

	
	// direct blitter memory acces (a 16mb memory block)
	// takes into account if rom font is activated
	inline uint8_t memory_read_8(uint32_t address)
	{
		if (((blit[(address & 0x00ff0000) >> 16].flags_0) & 0b10000000)
		    && !(address & 0x00008000)) {
			return ((uint8_t *)cbm_font)[address & 0x7fff];
		} else {
			return blit_memory[address & 0x00ffffff];
		}
	}
	
	inline void memory_write_8(uint32_t address, uint8_t value)
	{
		blit_memory[address & 0x00ffffff] = value;
	}
	
	// used from inside the machine (which can not access 16mb of flat memory)
	void indirect_memory_write_8(uint8_t address, uint8_t byte)
	{
		uint32_t temp_address = registers[0x0f] << 16 |
					registers[0x0e] << 8  |
					address;
		memory_write_8(temp_address, byte);
	}
	
	uint8_t indirect_memory_read_8(uint8_t address)
	{
		uint32_t temp_address = registers[0x0f] << 16 |
					registers[0x0e] << 8  |
					address;
		return memory_read_8(temp_address);
	}
	
	
	// blitter pointer access (256 * 8  = 2048 bytes)
	inline uint8_t descriptor_read_8(uint16_t address)
	{
		switch (address & 0x07) {
			case 0x00:
				return blit[(address & 0b11111111000) >> 3].flags_0;
			case 0x01:
				return blit[(address & 0b11111111000) >> 3].flags_1;
			case 0x02:
				return blit[(address & 0b11111111000) >> 3].get_size_in_tiles_log2();
			case 0x03:
				return blit[(address & 0b11111111000) >> 3].currently_unused;
			case 0x04:
				return (blit[(address & 0b11111111000) >> 3].foreground_color) & 0x00ff;
			case 0x05:
				return ((blit[(address & 0b11111111000) >> 3].foreground_color) & 0xff00) >> 8;
			case 0x06:
				return (blit[(address & 0b11111111000) >> 3].background_color) & 0x00ff;
			case 0x07:
				return ((blit[(address & 0b11111111000) >> 3].background_color) & 0xff00) >> 8;
			default:
				return 0x00;
		}
	}
	
	inline void descriptor_write_8(uint16_t address, uint8_t value)
	{
		uint16_t temp_word;
		
		switch (address & 0x07) {
			case 0x00:
				blit[(address & 0b11111111000) >> 3].flags_0 = value;
				break;
			case 0x01:
				blit[(address & 0b11111111000) >> 3].flags_1 = value;
				break;
			case 0x02:
				blit[(address & 0b11111111000) >> 3].set_size_in_tiles_log2(value);
				break;
			case 0x03:
				blit[(address & 0b11111111000) >> 3].currently_unused = value;
				break;
			case 0x04:
				temp_word = (blit[(address & 0b11111111000) >> 3].foreground_color) & 0xff00;
				blit[(address & 0b11111111000) >> 3].foreground_color = temp_word | value;
				break;
			case 0x05:
				temp_word = (blit[(address & 0b11111111000) >> 3].foreground_color) & 0x00ff;
				blit[(address & 0b11111111000) >> 3].foreground_color = temp_word | (value << 8);
				break;
			case 0x06:
				temp_word = (blit[(address & 0b11111111000) >> 3].background_color) & 0xff00;
				blit[(address & 0b11111111000) >> 3].background_color = temp_word | value;
				break;
			case 0x07:
				temp_word = (blit[(address & 0b11111111000) >> 3].background_color) & 0x00ff;
				blit[(address & 0b11111111000) >> 3].background_color = temp_word | (value << 8);
				break;
		}
	}
	
	void swap_buffers();

	// framebuffer pointers inside the virtual machine
	uint16_t *frontbuffer;
	uint16_t *backbuffer;
	
	void reset();
	void run(int no_of_cycles);
	//inline void make_idle() { blitter_state = IDLE; }
	
	struct blit_t *blit;	// 2048 bytes (256 * 8) for E64, another 2048 for host
	
	void set_clear_color(uint16_t color);
	void set_border_color(uint16_t color) { border_color = color; }
	void set_border_size(uint8_t size ) { border_size = size; }
	
	void clear_framebuffer();
	void draw_border();
	void draw_blit(blit_t *blit, int16_t x, int16_t y);
	
	inline bool busy() { return blitter_state == IDLE ? false : true; }
	
	// run cycles until not busy anymore
	inline void flush() {
		do run(1000); while (busy());
	}
};

}

#endif
