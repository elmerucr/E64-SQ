//  video.hpp
//  E64
//
//  Copyright © 2020 elmerucr. All rights reserved.

#include <SDL2/SDL.h>

#ifndef VIDEO_HPP
#define VIDEO_HPP

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
	
	//void init_palette();
public:
	video_t();
	~video_t();
	
	/* framebuffer pointers */
	uint16_t *framebuffer;
	
//	/* Palette, 4096 colors (3 x 4 bit) */
//	uint32_t *palette;
    
	// must be called upon machine reset
	void reset();
    
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
