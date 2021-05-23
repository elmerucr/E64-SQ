//  blit.cpp
//  E64
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

#include "blit.hpp"
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
				bitmap_mode	= blit[operations[tail].blit_no].flags_0 & 0b00000001 ? true : false;
				background      = blit[operations[tail].blit_no].flags_0 & 0b00000010 ? true : false;
				multicolor_mode = blit[operations[tail].blit_no].flags_0 & 0b00000100 ? true : false;
				color_per_tile  = blit[operations[tail].blit_no].flags_0 & 0b00001000 ? true : false;
				use_cbm_font    = blit[operations[tail].blit_no].flags_0 & 0b10000000 ? true : false;
		    
				// check flags 1
				double_width	= blit[operations[tail].blit_no].flags_1 & 0b00000001 ? 1 : 0;
				double_height   = blit[operations[tail].blit_no].flags_1 & 0b00000100 ? 1 : 0;
				hor_flip = blit[operations[tail].blit_no].flags_1 & 0b00010000 ? true : false;
				ver_flip = blit[operations[tail].blit_no].flags_1 & 0b00100000 ? true : false;

				width_in_tiles_log2 = blit[operations[tail].blit_no].get_size_in_tiles_log2() & 0b00000111;
				height_in_tiles_log2 = (blit[operations[tail].blit_no].get_size_in_tiles_log2() & 0b01110000) >> 4;
		    
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
				foreground_color = blit[operations[tail].blit_no].foreground_color;
				background_color = blit[operations[tail].blit_no].background_color;
				pixel_data = blit[operations[tail].blit_no].pixel_data;
				tile_data = blit[operations[tail].blit_no].tile_data;
				tile_color_data = blit[operations[tail].blit_no].tile_color_data;
				tile_background_color_data = blit[operations[tail].blit_no].tile_background_color_data;
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

void E64::blitter_ic::draw_blit(int blit_no, int16_t x, int16_t y)
{
	operations[head].type = BLIT;
	operations[head].blit_no = blit_no;
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

uint8_t E64::blitter_ic::read_byte(uint8_t address)
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
		default:
			return registers[address & 0x1f];
	}
}

void E64::blitter_ic::write_byte(uint8_t address, uint8_t byte)
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
					draw_blit(registers[0x01],
						  registers[0x04] | (registers[0x05] << 8),
						  registers[0x06] | (registers[0x07] << 8));
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
		default:
			registers[address & 0x1f] = byte;
	}
}
