//  blitter.cpp
//  E64
//
//  Copyright © 2020 elmerucr. All rights reserved.

#include "blitter.hpp"
#include "common.hpp"

/*
 * alpha_blend function takes the current color (destination, which is
 * also the destination) and the color that must be blended (source). It
 * returns the value of the blend which, normally, will be written to the
 * destination.
 * The ordering (from little endian perspective) seems strange: GBAR4444
 * Actually, it isn't: inside the virtual machine (big endian) it is
 * in ARGB4444 format. At first, this function seemed to drag down total
 * emulation speed. But, with optimizations (minimum -O2) turned on, it
 * is ok.
 *
 * The idea to use a function (and not a lookup table) comes from this website:
 * https://stackoverflow.com/questions/30849261/alpha-blending-using-table-lookup-is-not-as-fast-as-expected
 * Generally, lookup tables mess around with the cpu cache and don't speed up.
 *
 * In three steps a derivation (source is color to apply, destination
 * is the original color, a is alpha value):
 * (1) ((source * a) + (destination * (COLOR_MAX - a))) / COLOR_MAX
 * (2) ((source * a) - (destination * a) + (destination * COLOR_MAX)) / COLOR_MAX
 * (3) destination + (((source - destination) * a) / COLOR_MAX)
 */

/*
 * Update 2020-06-10, check:
 * https://stackoverflow.com/questions/12011081/alpha-blending-2-rgba-colors-in-c
 *
 * Calculate inv_alpha, then makes use of a bit shift, no divisions anymore. Also
 * bit shifts are not performed immediately after assigning the initial values,
 * only during the last step.
 *
 * (1) isolate alpha value (0 - max) and add 1
 * (2) calculate inverse alpha by taking (max+1) - alpha
 * (3) calculate the new individual channels:
 *      new = (alpha * source) + (inv_alpha * dest)
 * (4) bitshift the result to the right (normalize)
 *
 * Speeds up a little.
 */


// NEEDS WORK!!!
static void alpha_blend(uint16_t *destination, uint16_t *source)
{
	uint16_t r_dest, g_dest, b_dest;
	uint16_t a_src, a_src_inv, r_src, g_src, b_src;
    
	r_dest = (*destination & 0x000f);
	g_dest = (*destination & 0xf000);   // bitshift >>12 done in final step
	b_dest = (*destination & 0x0f00);   // bitshift >> 8 done in final step

	a_src = ((*source & 0x00f0) >> 4) + 1;
	r_src =  (*source & 0x000f);
	g_src =  (*source & 0xf000); // bitshift of >>12 is done in final step
	b_src =  (*source & 0x0f00); // bitshift of >> 8 is done in final step
    
	a_src_inv = 17 - a_src;

	r_dest = ((a_src * r_src) + (a_src_inv * r_dest)) >> 4;
	g_dest = ((a_src * g_src) + (a_src_inv * g_dest)) >> (4 + 12);
	b_dest = ((a_src * b_src) + (a_src_inv * b_dest)) >> (4 + 8);

	//  Anything being returned has always an alpha value of 0xf
	//  Note the format: gbar4444
	*destination = (g_dest << 12) | (b_dest << 8) | 0x00f0 | r_dest;
}

void E64::blitter_ic::reset()
{
	current_state = IDLE;

	head = 0;
	tail = 0;

	cycles_busy = 0;
	cycles_idle = 0;
}

inline void E64::blitter_ic::check_new_operation()
{
	if (head != tail) {
		switch (operations[tail].type) {
		case CLEAR_FRAMEBUFFER:
		    current_state = CLEARING_FRAMEBUFFER;
		    width_on_screen = VICV_PIXELS_PER_SCANLINE;
		    height_on_screen = VICV_SCANLINES;
		    total_no_of_pix = (width_on_screen * height_on_screen);
		    pixel_no = 0;
		    clear_color = (registers[6] | registers[7] << 8) | 0x00f0;
		    tail++;
		    break;
		case BLIT:
		    current_state = BLITTING;
		    
		    // set up the blitting finite state machine
		    
		    // check flags 0
		    bitmap_mode     = (operations[tail].this_blit.flags_0 & 0b00000001) ? true : false;
		    background      = (operations[tail].this_blit.flags_0 & 0b00000010) ? true : false;
		    multicolor_mode = (operations[tail].this_blit.flags_0 & 0b00000100) ? true : false;
		    color_per_tile  = (operations[tail].this_blit.flags_0 & 0b00001000) ? true : false;
		    
		    // check flags 1
		    double_width    = (operations[tail].this_blit.flags_1 & 0b00000001) ? 1 : 0;
		    double_height   = (operations[tail].this_blit.flags_1 & 0b00000100) ? 1 : 0;
		    hor_flip = (operations[tail].this_blit.flags_1 & 0b00010000) ? true : false;
		    ver_flip = (operations[tail].this_blit.flags_1 & 0b00100000) ? true : false;
		    
		    width_in_tiles_log2  = operations[tail].this_blit.size_in_tiles_log2  & 0b00000111;
		    height_in_tiles_log2 = (operations[tail].this_blit.size_in_tiles_log2 & 0b01110000) >> 4;
		    
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
		    
		    x = operations[tail].this_blit.x_low_byte |
			    operations[tail].this_blit.x_high_byte << 8;
		    y = operations[tail].this_blit.y_low_byte |
			    operations[tail].this_blit.y_high_byte << 8;
		    
		    foreground_color =
			operations[tail].this_blit.foreground_color__0__7       |
			operations[tail].this_blit.foreground_color__8_15 <<  8;
		    
		    background_color =
			operations[tail].this_blit.background_color__0__7       |
			operations[tail].this_blit.background_color__8_15 <<  8;
		    
		    pixel_data =
			operations[tail].this_blit.pixel_data__0__7       |
			operations[tail].this_blit.pixel_data__8_15 <<  8 |
			operations[tail].this_blit.pixel_data_16_23 << 16 |
			operations[tail].this_blit.pixel_data_24_31 << 24;
		    
		    tile_data =
			operations[tail].this_blit.tile_data__0__7       |
			operations[tail].this_blit.tile_data__8_15 <<  8 |
			operations[tail].this_blit.tile_data_16_23 << 16 |
			operations[tail].this_blit.tile_data_24_31 << 24;
		    
		    tile_color_data =
			operations[tail].this_blit.tile_color_data__0__7       |
			operations[tail].this_blit.tile_color_data__8_15 <<  8 |
			operations[tail].this_blit.tile_color_data_16_23 << 16 |
			operations[tail].this_blit.tile_color_data_24_31 << 24;
		    
		    tile_background_color_data =
			operations[tail].this_blit.tile_background_color_data__0__7       |
			operations[tail].this_blit.tile_background_color_data__8_15 <<  8 |
			operations[tail].this_blit.tile_background_color_data_16_23 << 16 |
			operations[tail].this_blit.tile_background_color_data_24_31 << 24;
		    
		    user_data =
			operations[tail].this_blit.user_data__0__7       |
			operations[tail].this_blit.user_data__8_15 <<  8 |
			operations[tail].this_blit.user_data_16_23 << 16 |
			operations[tail].this_blit.user_data_24_31 << 24;
		    
		    tail++;
		    
		    break;
		}
	}
}

void E64::blitter_ic::run(int no_of_cycles)
{
    while(no_of_cycles > 0)
    {
        no_of_cycles--;
        
        switch( current_state )
        {
            case IDLE:
                cycles_idle++;
		check_new_operation();
                break;
            case CLEARING_FRAMEBUFFER:
                cycles_busy++;
                
                if (!(pixel_no == total_no_of_pix)) {
                    machine.vicv->backbuffer[pixel_no] = clear_color;
                    pixel_no++;
                }
                else {
                    current_state = IDLE;
                }
                break;
            case BLITTING:
                cycles_busy++;
                
                if( !(pixel_no == total_no_of_pix) )
                {
                    scrn_x = x + (hor_flip ? (width_on_screen - (pixel_no & width_on_screen_mask) - 1) : (pixel_no & width_on_screen_mask) );
                    
                    if( scrn_x < VICV_PIXELS_PER_SCANLINE )                     // clipping check horizontally
                    {
                        scrn_y = y + (ver_flip ?
                            (height_on_screen - (pixel_no >> width_on_screen_log2) - 1) : (pixel_no >> width_on_screen_log2) );
                        
                        //if( (scrn_y >= 0) && (scrn_y < VICV_SCANLINES) )      // clipping check vertically
                        if( scrn_y < VICV_SCANLINES )      // clipping check vertically
                        {
                            /*  Transformation of pixel_no to take into account double width and/or height. After
                             *  this <complex> transformation, the normalized pixel_no points to a position in the
                             *  source material.
                             *  NEEDS WORK: document this transformation
                             */
                            normalized_pixel_no = (((pixel_no >> double_height) & ~width_on_screen_mask) | (pixel_no & width_on_screen_mask)) >> double_width;
                            
                            /*  Calculate the current x and y positions within the current blit source pixeldata */
                            x_in_blit = normalized_pixel_no & width_mask;
                            y_in_blit = normalized_pixel_no >> width_log2;
                            
                            tile_x = x_in_blit >> 3;
                            tile_y = y_in_blit >> 3;
                            
                            tile_number = tile_x + (tile_y << width_in_tiles_log2);
                            
                            tile_index = machine.mmu->ram[(tile_data + tile_number) & 0x00ffffff];
                            
                            /* Replace foreground and background colors if necessary */
                            if( color_per_tile )
                            {
                                foreground_color = machine.mmu->ram_as_words[((tile_color_data >> 1) + tile_number) & 0x007fffff];
                                
                                background_color = machine.mmu->ram_as_words[ ((tile_background_color_data >> 1) + tile_number) & 0x007fffff ];
                            }
                            
                            pixel_in_tile = (x_in_blit & 0b111) | ((y_in_blit & 0b111) << 3);
                            
                            /*  Pick the right pixel depending on bitmap mode or tile mode */
                            source_color = bitmap_mode ?
                                machine.mmu->ram_as_words[((pixel_data >> 1) + normalized_pixel_no) & 0x007fffff]
                                :
                                machine.mmu->ram_as_words[((pixel_data >> 1) + ((tile_index << 6) | pixel_in_tile) ) & 0x007fffff];
                            
                            /*  If the source color has an alpha value of higher than 0x0 (there is a pixel),
                             *  and we're not in multicolor mode, replace with foreground color.
                             *
                             *  If there's no alpha value (no pixel), and we have background 'on',
                             *  replace the color with background color.
                             *
                             *  At this stage, we have already checked for color per
                             *  tile, and if so, the value of foreground and respectively
                             *  background color have been replaced accordingly.
                             */
                            if( source_color & 0x00f0 )
                            {
                                if( !multicolor_mode ) source_color = foreground_color;
                            }
                            else
                            {
                                if( background ) source_color = background_color;
                            }
                            
                            /*  Finally, call the alpha blend function */
                            alpha_blend( &machine.vicv->backbuffer[scrn_x + (scrn_y * VICV_PIXELS_PER_SCANLINE)], &source_color );
                        }
                    }
                    pixel_no++;
                }
                else
                {
                    current_state = IDLE;
                }
                break;
        }
    }
}

uint8_t E64::blitter_ic::read_byte(uint8_t address)
{
	switch (address) {
	case 0x00:
		if (current_state == IDLE)
			return 0b00000000;
		else
			return 0b00000001;
		break;
	default:
		return registers[address];
		break;
	}
}

void E64::blitter_ic::write_byte(uint8_t address, uint8_t byte)
{
	switch (address) {
	case 0x00:
		if (byte & 0b00000001) { // add operation
			uint32_t ptr_to_blit_struct =
				(registers[2]<<24) |
				(registers[3]<<16) |
				(registers[4]<<8) |
				registers[5];
			if (ptr_to_blit_struct & 0x80000000) {
				operations[head].type = CLEAR_FRAMEBUFFER;
			} else {
				operations[head].type = BLIT;

				// make sure word aligned and equal or below 0xffffe0
				ptr_to_blit_struct &= 0xfffffe;
				if (ptr_to_blit_struct > 0xffffe0)
					ptr_to_blit_struct = 0xffffe0;

				// copy the structure into the operations list
				operations[head].this_blit = *(struct surface_blit *)&machine.mmu->ram[ptr_to_blit_struct];
			}
			head++;
		}
		break;
	default:
		registers[address] = byte;
		break;
	}
}

double E64::blitter_ic::fraction_busy()
{
	double percentage =
		(double)cycles_busy / (double)(cycles_busy + cycles_idle);
	cycles_busy = cycles_idle = 0;
	return percentage;
}
