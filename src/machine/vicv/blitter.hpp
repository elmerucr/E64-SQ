//  blitter.hpp
//  E64-SQ
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

/*
 * Blitter is able to copy data very fast from a video memory location to the
 * backbuffer (framebuffer). Copy operations run independently and can be added
 * to a FIFO linked list (blitter internally).
 */

#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

namespace E64
{

enum blitter_state_t {
	IDLE,
	CLEARING,
	BLITTING
};

/*
 * The next structure is a description of the surface blit how it appears
 * in the video memory of E64. Once a pointer to this structure is passed to
 * the blitter (addition of an operation), the structure is read and converted
 * into finite state machine data of the blitter.
 */

struct __attribute__((packed)) blit_t {
	/*  Flags 0
	 *
	 *  7 6 5 4 3 2 1 0
	 *          | | | |
	 *          | | | +-- Tile Mode (0) / Bitmap Mode (1)
	 *          | | +---- Background (0 = off, 1 = on)
	 *          | +------ Simple Color (0) / Multi Color (1)
	 *          +-------- Color per tile (0 = off, 1 = on)
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
    
	/*  Size of blit in tiles log2, 8 bit unsigned number.
	 *
	 *  7 6 5 4 3 2 1 0
	 *    | | |   | | |
	 *    | | |   +-+-+-- Low nibble  (using 3 bits)
	 *    | | |
	 *    +-+-+---------- High nibble (using 3 bits)
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
	uint8_t     size_in_tiles_log2;
	
	/*  Reserved byte for future purposes related to e.g. wrapping
	 */
	uint8_t     currently_unused;
    
	/*
	 * Contains the foreground color if single color.
	 */
	uint16_t foreground_color;
    
	/*
	 * Contains the background color if single color.
	 */
	uint16_t background_color;
};

struct surface_t {
	/*  Flags 0
	 *
	 *  7 6 5 4 3 2 1 0
	 *          | | | |
	 *          | | | +-- Tile Mode (0) / Bitmap Mode (1)
	 *          | | +---- Background (0 = off, 1 = on)
	 *          | +------ Simple Color (0) / Multi Color (1)
	 *          +-------- Color per tile (0 = off, 1 = on)
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
	uint8_t     size_in_tiles_log2;
	
	/*  Reserved byte for future purposes related to e.g. wrapping
	 */
	uint8_t     currently_unused;
    
	/*
	 * Contains the foreground color if single color.
	 */
	uint16_t foreground_color;
    
	/*
	 * Contains the background color if single color.
	 */
	uint16_t background_color;
    
	/*  pointer to pixels (can be tile pixels or bitmap pixels) */
	uint16_t *pixel_data;
    
	/*  32 bit pointer to start of tiles */
	uint8_t *tile_data;
    
	/*  32 bit pointer to start of tile color */
	uint16_t *tile_color_data;
    
	/*  32 bit pointer to start of tile background color */
	uint16_t *tile_background_color_data;
    
	/*
	 * And 4 bytes of user data. When this surface blit structure is
	 * used as a data structure for the current text screen, this user
	 * data may contain:
	 * - current cursor position (16 bit unsigned): first 2 bytes
	 * - current text color for new chars (16 bit unsigned): last 2 bytes
	 */
	uint32_t user_data;
};

enum operation_type {
	CLEAR,
	BLIT
};

struct operation {
	enum operation_type type;
	struct surface_t this_blit;
	int16_t x_pos;
	int16_t y_pos;
};

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	void reset();
	void run(int no_of_cycles);
	
	void set_clearcolor(uint16_t color);
	void add_clear_framebuffer();
	void add_blit(struct surface_t *blit, int16_t x, int16_t y);
	bool busy();
	
	enum blitter_state_t blitter_state;
	double fraction_busy(); /*  Returns the fraction of time the blitter was NOT idle */
private:
	uint8_t *blit_memory;
	struct surface_t *blit;	// 2048 bytes (256 * 8) for E64, another 2048 for host
	uint16_t *cbm_font;	// unpacked font
	
	/*
	 * Keeping track of busy and idle cycles. This way, it is possible
	 * to estimate the % of usage of the blitter chip. It is best to
	 * calculate this only once per frame.
	 */
	uint64_t cycles_busy;
	uint64_t cycles_idle;

	/*
	 * Circular buffer containing operations. If more than 256 operations
	 * would be written (unlikely) and unfinished, something will be
	 * overwritten.
	 */
	struct operation operations[256];
	uint8_t head;
	uint8_t tail;
    
	/* Finite state machine */
	bool        bitmap_mode;
	bool        background;
	bool        multicolor_mode;
	bool        color_per_tile;
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

	int16_t x;
	int16_t y;
	
	uint16_t scrn_x;            // final screen x for the current pixel
	uint16_t scrn_y;            // final screen y for the current pixel
	
	uint16_t x_in_blit;
	uint16_t y_in_blit;
	
	uint16_t tile_x;
	uint16_t tile_y;
	
	
	uint16_t    tile_number;
	uint8_t     tile_index;
	uint16_t    current_background_color;
	uint8_t     pixel_in_tile;
	
	uint16_t source_color;
	
	uint16_t width_mask;
	uint16_t width_on_screen_mask;
    
	uint16_t foreground_color;
	uint16_t background_color;
    
	uint32_t pixel_data;
	uint32_t tile_data;
	uint32_t tile_color_data;
	uint32_t tile_background_color_data;
	uint32_t user_data;
	
	inline void check_new_operation();
};

}

#endif
