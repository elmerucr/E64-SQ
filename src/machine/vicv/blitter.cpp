//  blitter.cpp
//  E64-SQ
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

#include "blitter.hpp"
#include "common.hpp"

/*
 * The alpha_blend function takes the current color (destination, which is
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

/*
 * Update 2021-03-04, adapted for little endian usage
 */

//static void alpha_blend(uint16_t *destination, uint16_t *source)
//{
//	uint16_t r_dest, g_dest, b_dest;
//	uint16_t a_src, a_src_inv, r_src, g_src, b_src;
//
//	r_dest = (*destination & 0x0f00);   // bitshift >>8 done in final step
//	g_dest = (*destination & 0x00f0);   // bitshift >>4 done in final step
//	b_dest = (*destination & 0x000f);
//
//	a_src = ((*source & 0xf000) >> 12) + 1;
//	r_src =  (*source & 0x0f00);   // bitshift >>8 done in final step
//	g_src =  (*source & 0x00f0);   // bitshift >>4 done in final step
//	b_src =  (*source & 0x000f);
//
//	a_src_inv = 17 - a_src;
//
//	r_dest = ((a_src * r_src) + (a_src_inv * r_dest)) >> (4 + 8);
//	g_dest = ((a_src * g_src) + (a_src_inv * g_dest)) >> (4 + 4);
//	b_dest = ((a_src * b_src) + (a_src_inv * b_dest)) >> 4;
//
//	// Anything being returned has always an alpha value of 0xf
//	*destination = 0xf000 | (r_dest << 8) | (g_dest << 4) | b_dest;
//}

static void alpha_blend(uint16_t *destination, uint16_t *source)
{
	uint16_t r_dest, g_dest, b_dest;
	uint16_t a_src, a_src_inv, r_src, g_src, b_src;
    
	r_dest = (*destination & 0x0f00) >> 8;
	g_dest = (*destination & 0x00f0) >> 4;
	b_dest = (*destination & 0x000f);

	a_src = ((*source & 0xf000) >> 12) + 1;
	r_src =  (*source & 0x0f00) >> 8;
	g_src =  (*source & 0x00f0) >> 4;
	b_src =  (*source & 0x000f);
    
	a_src_inv = 17 - a_src;

	r_dest = ((a_src * r_src) + (a_src_inv * r_dest)) >> 4;
	g_dest = ((a_src * g_src) + (a_src_inv * g_dest)) >> 4;
	b_dest = ((a_src * b_src) + (a_src_inv * b_dest)) >> 4;

	// Anything being returned has always an alpha value of 0xf
	*destination = 0xf000 | (r_dest << 8) | (g_dest << 4) | b_dest;
}

void E64::blitter_ic::reset()
{
	blitter_state = IDLE;

	head = 0;
	tail = 0;

	cycles_busy = 0;
	cycles_idle = 0;
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
			case BLIT:
				blitter_state = BLITTING;
		    
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
		    
				x = operations[tail].x_pos;
				y = operations[tail].y_pos;
				foreground_color = operations[tail].this_blit.foreground_color;
				background_color = operations[tail].this_blit.background_color;
				pixel_data = (uint32_t)((uint64_t)operations[tail].this_blit.pixel_data - (uint64_t)machine.mmu->ram);
				tile_data = (uint32_t)((uint64_t)operations[tail].this_blit.tile_data - (uint64_t)machine.mmu->ram);
				tile_color_data = (uint32_t)((uint64_t)operations[tail].this_blit.tile_color_data - (uint64_t)machine.mmu->ram);
				tile_background_color_data = (uint32_t)((uint64_t)operations[tail].this_blit.tile_background_color_data - (uint64_t)machine.mmu->ram);
				user_data = operations[tail].this_blit.user_data;
		    
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
			cycles_idle++;
			check_new_operation();
			break;
		case CLEARING:
			cycles_busy++;
			if (!(pixel_no == total_no_of_pix)) {
				machine.vicv->backbuffer[pixel_no] = clear_color;
				pixel_no++;
			} else {
				blitter_state = IDLE;
			}
			break;
		case BLITTING:
			cycles_busy++;
                
			if (!(pixel_no == total_no_of_pix)) {
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
                            if( source_color & 0xf000 )
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
                    blitter_state = IDLE;
                }
                break;
        }
    }
}

bool E64::blitter_ic::busy()
{
	return blitter_state == IDLE ? false : true;
}

double E64::blitter_ic::fraction_busy()
{
	double fraction =
		(double)cycles_busy / (double)(cycles_busy + cycles_idle);
	cycles_busy = cycles_idle = 0;
	return fraction;
}

void E64::blitter_ic::set_clearcolor(uint16_t color)
{
	clear_color = color | 0xf000;
}

void E64::blitter_ic::clear_framebuffer()
{
	operations[head].type = CLEAR;
	head++;
}

void E64::blitter_ic::add_blit(struct surface_t *blit, int16_t x, int16_t y)
{
	operations[head].type = BLIT;
	operations[head].this_blit = *blit;
	operations[head].x_pos = x;
	operations[head].y_pos = y;
	head++;
}
