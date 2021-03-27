//  video.hpp
//  E64
//
//  Copyright © 2020 elmerucr. All rights reserved.

#include <SDL2/SDL.h>

#ifndef VIDEO_HPP
#define VIDEO_HPP

/*
 * The alpha_blend function takes the current color (destination, which is
 * also the destination) and the color that must be blended (source). It
 * returns the value of the blend which, normally, will be written to the
 * destination.
 * At first, this function seemed to drag down total emulation speed. But, with
 * optimizations (minimum -O2) turned on, it is ok.
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
 * Calculate inv_alpha, then makes use of a bit shift, no divisions anymore.
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

inline void alpha_blend(uint16_t *destination, uint16_t *source)
{
	uint16_t a_dest, r_dest, g_dest, b_dest;
	uint16_t a_src , r_src , g_src , b_src;
	uint16_t a_src_inv;

	a_dest = (*destination & 0xf000) >> 12;
	r_dest = (*destination & 0x0f00) >>  8;
	g_dest = (*destination & 0x00f0) >>  4;
	b_dest = (*destination & 0x000f);

	a_src = ((*source & 0xf000) >> 12) + 1;
	r_src =  (*source & 0x0f00) >> 8;
	g_src =  (*source & 0x00f0) >> 4;
	b_src =  (*source & 0x000f);
    
	a_src_inv = 17 - a_src;
	
	a_dest = (a_dest >= (a_src-1)) ? a_dest : (a_src-1);
	r_dest = ((a_src * r_src) + (a_src_inv * r_dest)) >> 4;
	g_dest = ((a_src * g_src) + (a_src_inv * g_dest)) >> 4;
	b_dest = ((a_src * b_src) + (a_src_inv * b_dest)) >> 4;

//	// Anything returned, has an alpha value of 0xf
//	*destination = 0xf000 | (r_dest << 8) | (g_dest << 4) | b_dest;
	*destination = (a_dest << 12) | (r_dest << 8) | (g_dest << 4) | b_dest;
}

namespace E64 {

struct window_size
{
    uint16_t x;
    uint16_t y;
};

class video_t {
private:
	const struct window_size window_sizes[5] = {
		{  512, 288 },
		{  640, 360 },
		{  960, 540 },
		{ 1024, 576 },
		{ 1280, 720 }
	};
	SDL_Window *window;
	SDL_Renderer *renderer;
	bool vsync;
	SDL_Texture *texture;
	uint8_t current_window_size;
	bool fullscreen;
	int window_width;
	int window_height;
	
	uint16_t *framebuffer;
public:
	video_t();
	~video_t();

	void clear_frame_buffer();
	void merge_down_buffer(uint16_t *buffer);
	void update_screen();
	void update_title();
	void reset_window_size();
	void increase_window_size();
	void decrease_window_size();
	void toggle_fullscreen();
    
	// getters
	uint16_t current_window_width() { return window_sizes[current_window_size].x; }
	uint16_t current_window_height() { return window_sizes[current_window_size].y; }
	inline bool vsync_enabled() { return vsync; }
	inline bool vsync_disabled() { return !vsync; }
};

}

#endif
