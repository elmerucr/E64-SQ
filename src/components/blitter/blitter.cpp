//  blitter.cpp
//  E64
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

#include "blitter.hpp"
#include "rom.hpp"
#include "common.hpp"

E64::blitter_ic::blitter_ic()
{
	fb0 = new uint16_t[VICV_PIXELS_PER_SCANLINE * VICV_SCANLINES];
	fb1 = new uint16_t[VICV_PIXELS_PER_SCANLINE * VICV_SCANLINES];
	
	blit_memory = new uint8_t[256 * 65536];	// 16mb
	
	// fill blit memory with something
	for (int i=0; i < (256 * 65536); i++) {
		if (i & 0b1) {
			blit_memory[i] = (i & 0xff0000) >> 16;
		} else {
			blit_memory[i] = (i & 0x00ff00) >> 8;
		}
	}
	
	blit = new struct blit_t[256];
	cbm_font = new uint16_t[256 * 64];
	
	for (int i=0; i<256; i++) {
		blit[i].flags_0 = 0;
		blit[i].flags_1 = 0;
		blit[i].set_size_in_tiles_log2(0);
		blit[i].currently_unused = 0;
		blit[i].foreground_color = 0;
		blit[i].background_color = 0;
		blit[i].pixel_data                 = (uint16_t *)&blit_memory[(i << 16) | 0x0000]; // 32k block
		blit[i].tile_data                  = (uint8_t  *)&blit_memory[(i << 16) | 0x8000]; // 4k  block
		blit[i].tile_color_data            = (uint16_t *)&blit_memory[(i << 16) | 0xc000]; // 8k  block
		blit[i].tile_background_color_data = (uint16_t *)&blit_memory[(i << 16) | 0xe000]; // 8k  block
	}
	
	// expand characters
	uint16_t *dest = cbm_font;
	for (int i=0; i<2048; i++) {
		uint8_t byte = cbm_cp437_font[i];
		uint8_t count = 8;
		while (count--) {
			*dest = (byte & 0b10000000) ? C64_GREY : 0x0000;
			dest++;
			byte = byte << 1;
		}
	}
}

E64::blitter_ic::~blitter_ic()
{
	delete [] cbm_font;
	delete [] blit;
	delete [] blit_memory;
	
	delete [] fb1;
	delete [] fb0;
}


void E64::blitter_ic::reset()
{
	blitter_state = IDLE;

	head = 0;
	tail = 0;
	
	for (int i=0; i<VICV_PIXELS_PER_SCANLINE*VICV_SCANLINES; i++) {
		fb0[i] = 0xf222;
		fb1[i] = 0xf222;
	}
	
	frontbuffer = fb0;
	backbuffer  = fb1;
}

inline void E64::blitter_ic::check_new_operation()
{
	if (head != tail) {
		switch (operations[tail].type) {
			case CLEAR:
				blitter_state = CLEARING;
				width_on_screen = VICV_PIXELS_PER_SCANLINE;
				height_on_screen = VICV_SCANLINES;
				total_no_of_pix = (width_on_screen * height_on_screen);
				pixel_no = 0;
				tail++;
				break;
			case BORDER:
				blitter_state = DRAW_BORDER;
				width_on_screen = VICV_PIXELS_PER_SCANLINE;
				height_on_screen = border_size;
				total_no_of_pix = (width_on_screen * height_on_screen);
				pixel_no = 0;
				tail++;
				break;
			case BLIT:
				blitter_state = BLITTING;

				// check flags 0
				bitmap_mode	= operations[tail].blit_pointer->flags_0 & 0b00000001 ? true : false;
				background      = operations[tail].blit_pointer->flags_0 & 0b00000010 ? true : false;
				multicolor_mode = operations[tail].blit_pointer->flags_0 & 0b00000100 ? true : false;
				color_per_tile  = operations[tail].blit_pointer->flags_0 & 0b00001000 ? true : false;
				use_cbm_font    = operations[tail].blit_pointer->flags_0 & 0b10000000 ? true : false;
		    
				// check flags 1
				double_width	= operations[tail].blit_pointer->flags_1 & 0b00000001 ? 1 : 0;
				double_height   = operations[tail].blit_pointer->flags_1 & 0b00000100 ? 1 : 0;
				hor_flip = operations[tail].blit_pointer->flags_1 & 0b00010000 ? true : false;
				ver_flip = operations[tail].blit_pointer->flags_1 & 0b00100000 ? true : false;

				width_in_tiles_log2 = operations[tail].blit_pointer->get_size_in_tiles_log2() & 0b00000111;
				height_in_tiles_log2 = (operations[tail].blit_pointer->get_size_in_tiles_log2() & 0b01110000) >> 4;
		    
				width_log2 = width_in_tiles_log2 + 3;
				height_log2 = height_in_tiles_log2 + 3;
		    
				width_on_screen_log2  = width_log2 + double_width;
				height_on_screen_log2 = height_log2 + double_height;
		    
				width = 1 << width_log2;
				height = 1 << height_log2;
		    
				width_on_screen  = 1 << width_on_screen_log2;
				height_on_screen = 1 << height_on_screen_log2;
		    
				width_mask = width - 1;
				width_on_screen_mask = width_on_screen - 1;
				pixel_no = 0;
				total_no_of_pix = width_on_screen * height_on_screen;
				x = operations[tail].x_pos;
				y = operations[tail].y_pos;
				foreground_color = operations[tail].blit_pointer->foreground_color;
				background_color = operations[tail].blit_pointer->background_color;
				pixel_data = operations[tail].blit_pointer->pixel_data;
				tile_data = operations[tail].blit_pointer->tile_data;
				tile_color_data = operations[tail].blit_pointer->tile_color_data;
				tile_background_color_data = operations[tail].blit_pointer->tile_background_color_data;
				tail++;
				break;
		}
	}
}

void E64::blitter_ic::run(int no_of_cycles)
{
	while (no_of_cycles > 0) {
		no_of_cycles--;

		switch (blitter_state) {
		case IDLE:
			check_new_operation();
			break;
		case CLEARING:
			if (!(pixel_no == total_no_of_pix)) {
				backbuffer[pixel_no] = clear_color;
				pixel_no++;
			} else {
				blitter_state = IDLE;
			}
			break;
		case DRAW_BORDER:
			if (!(pixel_no == total_no_of_pix)) {
				alpha_blend(&backbuffer[pixel_no], &border_color);
				alpha_blend(&backbuffer[(VICV_TOTAL_PIXELS-1) - pixel_no], &border_color);
				pixel_no++;
			} else {
				blitter_state = IDLE;
			}
			break;
		case BLITTING:
			if (!(pixel_no == total_no_of_pix)) {
				scrn_x = x + (hor_flip ? (width_on_screen - (pixel_no & width_on_screen_mask) - 1) : (pixel_no & width_on_screen_mask) );
                    
				if (scrn_x < VICV_PIXELS_PER_SCANLINE) {        // clipping check horizontally
					scrn_y = y + (ver_flip ?
						      (height_on_screen - (pixel_no >> width_on_screen_log2) - 1) : (pixel_no >> width_on_screen_log2) );
                        
					if (scrn_y < VICV_SCANLINES) {     // clipping check vertically
                            /*
                             * Transformation of pixel_no to take into account double width and/or height. After
                             * this <complex> transformation, the normalized pixel_no points to a position in the
                             * source material.
                             * NEEDS WORK: document this transformation
                             */
                            normalized_pixel_no = (((pixel_no >> double_height) & ~width_on_screen_mask) | (pixel_no & width_on_screen_mask)) >> double_width;
                            
                            /* Calculate the current x and y positions within the current blit source pixeldata */
                            x_in_blit = normalized_pixel_no & width_mask;
                            y_in_blit = normalized_pixel_no >> width_log2;
                            
                            tile_x = x_in_blit >> 3;
                            tile_y = y_in_blit >> 3;
                            
                            tile_number = tile_x + (tile_y << width_in_tiles_log2);
                            
				tile_index = tile_data[tile_number & 0x7fff];
                            
                            /* Replace foreground and background colors if necessary */
                            if (color_per_tile) {
				    foreground_color = tile_color_data[tile_number & 0x7ff];
				    background_color = tile_background_color_data[tile_number & 0x7ff];
                            }
                            
                            pixel_in_tile = (x_in_blit & 0b111) | ((y_in_blit & 0b111) << 3);
                            
				/*
				 * Pick the right pixel depending on bitmap mode or tile mode,
				 * and based on cbm_font or not
				 */
				if (use_cbm_font) {
					source_color = bitmap_mode ?
					cbm_font[normalized_pixel_no & 0x3fff] : cbm_font[((tile_index << 6) | pixel_in_tile) & 0x3fff];
				} else {
                            source_color = bitmap_mode ?
				pixel_data[normalized_pixel_no & 0x3fff] : pixel_data[((tile_index << 6) | pixel_in_tile) & 0x3fff];
				}
                            
                            /*
			     * If the source color has an alpha value of higher than 0x0 (there is a pixel),
                             * and we're not in multicolor mode, replace with foreground color.
                             *
                             * If there's no alpha value (no pixel), and we have background 'on',
                             * replace the color with background color.
                             *
                             * At this stage, we have already checked for color per
                             * tile, and if so, the value of foreground and respectively
                             * background color have been replaced accordingly.
                             */
                            if (source_color & 0xf000) {
                                if (!multicolor_mode) source_color = foreground_color;
                            } else {
                                if (background) source_color = background_color;
                            }
                            
                            /* Finally, call the alpha blend function */
                            alpha_blend(&backbuffer[scrn_x + (scrn_y * VICV_PIXELS_PER_SCANLINE)], &source_color);
                        }
                    }
                    pixel_no++;
                } else {
                    blitter_state = IDLE;
                }
                break;
        }
    }
}

void E64::blitter_ic::set_clear_color(uint16_t color)
{
//	clear_color = color | 0xf000;
	clear_color = color;
}

void E64::blitter_ic::clear_framebuffer()
{
	operations[head].type = CLEAR;
	head++;
}

void E64::blitter_ic::draw_border()
{
	operations[head].type = BORDER;
	head++;
}

void E64::blitter_ic::draw_blit(blit_t *blit, int16_t x, int16_t y)
{
	operations[head].type = BLIT;
	operations[head].blit_pointer = blit;
	operations[head].x_pos = x;
	operations[head].y_pos = y;
	head++;
}

void E64::blitter_ic::swap_buffers()
{
	uint16_t *tempbuffer = frontbuffer;
	frontbuffer = backbuffer;
	backbuffer = tempbuffer;
}

uint8_t E64::blitter_ic::io_read_8(uint8_t address)
{
	switch (address & 0x1f) {
		case 0x00:
			return 0;
		case 0x02:
			return border_size;
		case 0x08:
			return clear_color & 0xff;
		case 0x09:
			return (clear_color & 0xff00) >> 8;
		case 0x0a:
			return border_color & 0xff;
		case 0x0b:
			return (border_color & 0xff00) >> 8;
		case 0x10:
			return blit[registers[0x01]].tiles & 0xff;
		case 0x11:
			return (blit[registers[0x01]].tiles & 0xff00) >> 8;
		case 0x12:
			// cursor position of current blit (reg 0x01), low byte
			return blit[registers[0x01]].cursor_position & 0xff;
		case 0x13:
			// cursor position of current blit (reg 0x01), high byte
			return (blit[registers[0x01]].cursor_position & 0xff00) >> 8;
		default:
			return registers[address & 0x1f];
	}
}

void E64::blitter_ic::io_write_8(uint8_t address, uint8_t byte)
{
	switch (address & 0x1f) {
		case 0x00:
			switch (byte) {
				case 0b00000001:
					swap_buffers();
					break;
				case 0b00000010:
					clear_framebuffer();
					break;
				case 0b00000100:
					draw_border();
					break;
				case 0b00001000:
					draw_blit(&blit[registers[0x01]],
						  registers[0x04] | (registers[0x05] << 8),
						  registers[0x06] | (registers[0x07] << 8));
					break;
				case 0b00010000:
					// reset cursor position
					blit[registers[0x01]].cursor_position = 0;
					break;
				case 0b00010001:
					break;
				default:
					break;
			}
			break;
		case 0x02:
			border_size = byte;
			break;
		case 0x08:
			clear_color = (clear_color & 0xff00) | byte;
			break;
		case 0x09:
			clear_color = (clear_color & 0x00ff) | (byte << 8);
			break;
		case 0x0a:
			border_color = (border_color & 0xff00) | byte;
			break;
		case 0x0b:
			border_color = (border_color & 0x00ff) | (byte << 8);
			break;
		case 0x12:
			blit[registers[0x01]].cursor_position =
				(blit[registers[0x01]].cursor_position & 0xff00) | byte;
			break;
		case 0x13:
			blit[registers[0x01]].cursor_position =
				(blit[registers[0x01]].cursor_position & 0x00ff) | (byte << 8);
			break;
		default:
			registers[address & 0x1f] = byte;
	}
}

void E64::blit_t::terminal_init(uint8_t flags_0, uint8_t flags_1,
				uint8_t size_in_tiles_log2, uint16_t foreground_color,
				uint16_t background_color)
{
	this->flags_0 = flags_0;
	this->flags_1 = flags_1;
	set_size_in_tiles_log2(size_in_tiles_log2);
	this->foreground_color = foreground_color;
	this->background_color = background_color;
}

void E64::blit_t::clear()
{
	for (size_t i=0; i < tiles; i++) {
		tile_data[i] = ' ';
		tile_color_data[i] = foreground_color;
		tile_background_color_data[i] = background_color;
	}
	
	cursor_position = 0;
	
	cursor_interval = 20; 	// 0.33s (if timer @ 60Hz)
	cursor_countdown = 0;
	cursor_blinking = false;
}

void E64::blit_t::putsymbol(char symbol)
{
	tile_data[cursor_position] = symbol;
	tile_color_data[cursor_position] = foreground_color;
	tile_background_color_data[cursor_position] = background_color;
	cursor_position++;
	if (cursor_position >= tiles) {
		add_bottom_row();
		cursor_position -= columns;
	}
}

int E64::blit_t::putchar(int character)
{
	uint8_t result = (uint8_t)character;
	switch (result) {
		case '\r':
			cursor_position -= cursor_position % columns;
			break;
		case '\n':
			cursor_position -= cursor_position % columns;
			if ((cursor_position / columns) == (rows - 1)) {
				    add_bottom_row();
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

int E64::blit_t::puts(const char *text)
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

int E64::blit_t::printf(const char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	int number = vsnprintf(buffer, 1024, format, args);
	va_end(args);
	puts(buffer);
	return number;
}

void E64::blit_t::prompt()
{
	putchar('\n');
}

void E64::blit_t::activate_cursor()
{
	cursor_original_char = tile_data[cursor_position];
	cursor_original_color = tile_color_data[cursor_position];
	cursor_original_background_color = tile_background_color_data[cursor_position];
	cursor_blinking = true;
	cursor_countdown = 0;
}

void E64::blit_t::deactivate_cursor()
{
	cursor_blinking = false;
	tile_data[cursor_position] = cursor_original_char;
	tile_color_data[cursor_position] = cursor_original_color;
	tile_background_color_data[cursor_position] = cursor_original_background_color;
}

void E64::blit_t::cursor_left()
{
	uint16_t min_pos = 0;
	if (cursor_position > min_pos) cursor_position--;
}

void E64::blit_t::cursor_right()
{
	cursor_position++;
	if (cursor_position > tiles - 1) {
		add_bottom_row();
		cursor_position -= columns;
	}
}

void E64::blit_t::cursor_up()
{
	cursor_position -= columns;

	if (cursor_position >= tiles) {
		uint32_t address;
	
		switch (check_output(true, &address)) {
			case E64::NOTHING:
				add_top_row();
				break;
			case E64::ASCII:
				add_top_row();
				hud.memory_dump((address-8) & (RAM_SIZE - 1), 1);
				break;
			case E64::BLITTER:
				add_top_row();
				hud.blit_memory_dump((address - 8) & 0xffffff, 1);
				break;
		}
	}
}

void E64::blit_t::cursor_down()
{
	cursor_position += columns;
	
	// cursor out of current screen?
	if (cursor_position >= tiles) {
		uint32_t address;
	
		switch (check_output(false, &address)) {
			case E64::NOTHING:
				add_bottom_row();
				cursor_position -= columns;
				break;
			case E64::ASCII:
				add_bottom_row();
				cursor_position -= columns;
				hud.memory_dump((address+8) & (RAM_SIZE - 1), 1);
				break;
			case E64::BLITTER:
				add_bottom_row();
				cursor_position -= columns;
				hud.blit_memory_dump((address + 8) & 0xffffff, 1);
				break;
		}
	}
}

void E64::blit_t::backspace()
{
	uint16_t pos = cursor_position;
	uint16_t min_pos = 0;

	if (pos > min_pos) {
		cursor_position--;
		while (pos % columns) {
			tile_data[pos - 1] = tile_data[pos];
			tile_color_data[pos - 1] = tile_color_data[pos];
			tile_background_color_data[pos - 1] = tile_background_color_data[pos];
			pos++;
		}
		tile_data[pos - 1] = ' ';
		tile_color_data[pos - 1] = foreground_color;
		tile_background_color_data[pos - 1] = background_color;
	}
}

void E64::blit_t::add_bottom_row()
{
	uint16_t no_of_tiles_to_move = tiles - columns;

	for (size_t i=0; i < no_of_tiles_to_move; i++) {
		tile_data[i] = tile_data[i + columns];
		tile_color_data[i] = tile_color_data[i + columns];
		tile_background_color_data[i] = tile_background_color_data[i + columns];
	}
	for (size_t i=no_of_tiles_to_move; i < tiles; i++) {
		tile_data[i] = ' ';
		tile_color_data[i] = foreground_color;
		tile_background_color_data[i] = background_color;
	}
}

void E64::blit_t::add_top_row()
{
	cursor_position += columns;
	for (int i=tiles-1; i >= (cursor_position - get_current_column()) + columns; i--) {
		tile_data[i] = tile_data[i - columns];
		tile_color_data[i] = tile_color_data[i - columns];
		tile_background_color_data[i] = tile_background_color_data[i - columns];
	}
	uint16_t start_pos = cursor_position - get_current_column();
	for (int i=0; i<columns; i++) {
		tile_data[start_pos] = ASCII_SPACE;
		tile_color_data[start_pos] = foreground_color;
		tile_background_color_data[start_pos] = background_color;
		start_pos++;
	}
}

enum E64::output_type E64::blit_t::check_output(bool top_down, uint32_t *address)
{
	enum output_type output = NOTHING;
	
	for (int i = 0; i < tiles; i += columns) {
		if (tile_data[i] == ':') {
			output = ASCII;
			char potential_address[5];
			for (int j=0; j<4; j++) {
				potential_address[j] =
				tile_data[i+1+j];
			}
			potential_address[4] = 0;
			hud.hex_string_to_int(potential_address, address);
			if (top_down) break;
		} else if (tile_data[i] == ';') {
			output = BLITTER;
			char potential_address[7];
			for (int j=0; j<6; j++) {
				potential_address[j] =
				tile_data[i+1+j];
			}
			potential_address[6] = 0;
			hud.hex_string_to_int(potential_address, address);
			if (top_down) break;
		}
	}
	return output;
}

void E64::blit_t::process_cursor_state()
{
	if (cursor_blinking == true) {
		if (cursor_countdown == 0) {
			tile_data[cursor_position] ^= 0x80;
			cursor_countdown += cursor_interval;
		}
		cursor_countdown--;
	}
}

char *E64::blit_t::enter_command()
{
	uint16_t start_of_line = cursor_position - (cursor_position % columns);
	for (size_t i = 0; i < columns; i++) {
		command_buffer[i] = tile_data[start_of_line + i];
	}
	size_t i = columns - 1;
	while (command_buffer[i] == ' ') i--;
	command_buffer[i + 1] = 0;
	
	return command_buffer;
}
