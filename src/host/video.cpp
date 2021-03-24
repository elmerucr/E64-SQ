//  video.cpp
//  E64
//
//  Copyright © 2020 elmerucr. All rights reserved.

#include "video.hpp"
#include "common.hpp"

E64::video_t::video_t()
{
	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	printf("[SDL] compiled against SDL version %d.%d.%d\n",
	       compiled.major, compiled.minor, compiled.patch);
	printf("[SDL] linked against SDL version %d.%d.%d\n",
	       linked.major, linked.minor, linked.patch);

	char *base_path = SDL_GetBasePath();
	printf("[SDL] base path is: %s\n", base_path);
	SDL_free(base_path);

	char *pref_path = SDL_GetPrefPath("elmerucr", "E64-II");
	printf("[SDL] pref path is: %s\n", pref_path);
	SDL_free(pref_path);

	SDL_Init(SDL_INIT_VIDEO);

	// print the list of video backends
	int num_video_drivers = SDL_GetNumVideoDrivers();
	printf("[SDL Display] %d video backend(s) compiled into SDL: ",
	       num_video_drivers);
	for (int i=0; i<num_video_drivers; i++)
		printf(" \'%s\' ", SDL_GetVideoDriver(i));
	printf("\n");
	printf("[SDL Display] now using backend '%s'\n", SDL_GetCurrentVideoDriver());

	current_window_size = 3;
	fullscreen = false;

	/*
	 * Create window - title will be set later on by E64::sdl2_update_title()
	 * Note: Usage of SDL_WINDOW_ALLOW_HIGHDPI actually helps: interpolation
	 * of pixels at unlogical window sizes looks a lot better!
	 */
	window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  window_sizes[current_window_size].x,
				  window_sizes[current_window_size].y,
				  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
				  SDL_WINDOW_ALLOW_HIGHDPI);
    
	SDL_GetWindowSize(window, &window_width, &window_height);
	printf("[SDL Display] window dimensions are %u x %u pixels\n",
	       window_width, window_height);
	
	update_title();
    
	/* Create renderer and link it to window */

	SDL_DisplayMode current_mode;

	SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window), &current_mode);

	printf("[SDL Display] current desktop dimensions: %i x %i\n",
	       current_mode.w, current_mode.h);

	printf("[SDL Display] refresh rate of current display is %iHz\n",
	       current_mode.refresh_rate);
    
	if (current_mode.refresh_rate == FPS) {
		printf("[SDL Display] this is equal to the FPS of E64-II, trying for vsync\n");
		renderer = SDL_CreateRenderer(window, -1,
					      SDL_RENDERER_ACCELERATED |
					      SDL_RENDERER_PRESENTVSYNC);
	} else {
		printf("[SDL Display] this differs from the FPS of E64-II, going for software FPS\n");
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}

	//  setting the logical size fixes aspect ratio
	//SDL_RenderSetLogicalSize(renderer, VICV_PIXELS_PER_SCANLINE, VICV_SCANLINES);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	SDL_RendererInfo current_renderer;
	SDL_GetRendererInfo(renderer, &current_renderer);
	vsync = (current_renderer.flags & SDL_RENDERER_PRESENTVSYNC) ? true : false;

	printf("[SDL Renderer Name] %s\n", current_renderer.name);
	printf("[SDL Renderer] %saccelerated\n",
	       (current_renderer.flags & SDL_RENDERER_ACCELERATED) ? "" : "not ");
	printf("[SDL Renderer] vsync is %s\n", vsync ? "enabled" : "disabled");

	// create a texture that is able to refresh very frequently
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444,
				    SDL_TEXTUREACCESS_STREAMING,
				    VICV_PIXELS_PER_SCANLINE, VICV_SCANLINES);

	// make sure mouse cursor isn't visible
	SDL_ShowCursor(SDL_DISABLE);

	// prepare screen buffer
	framebuffer = new uint16_t[VICV_PIXELS_PER_SCANLINE*VICV_SCANLINES];
	//buffer_1 = new uint32_t[VICV_PIXELS_PER_SCANLINE*VICV_SCANLINES];

	//init_palette();
}

E64::video_t::~video_t()
{
	//delete [] palette;
//	delete [] monitor_framebuffer;
	delete [] framebuffer;
	framebuffer = nullptr;
//	monitor_framebuffer = nullptr;
	//palette = nullptr;

	printf("[SDL] cleaning up video\n");
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void E64::video_t::reset()
{
	for (int i=0; i<VICV_PIXELS_PER_SCANLINE*VICV_SCANLINES; i++)
		framebuffer[i] = 0xf222;
}

//void E64::video_t::init_palette()
//{
//	/*
//	 * Prepare the 12 bit color palette and fill it with the right colors.
//	 * Index in the array is the actual 12 bit color, the 32 bit value is
//	 * the color in host space.
//	 */
//	palette = new uint32_t[0x10000];
//	for (int i = 0x0; i<0x10000; i++) {
//		uint8_t red   = (i & 0x0f00) >> 8;
//		uint8_t green = (i & 0x00f0) >> 4;
//		uint8_t blue  = (i & 0x000f);
//
//		palette[i] =	0xff000000 |
//				((red * 0x11) << 16) |
//				((green * 0x11) << 8) |
//				(blue * 0x11);
//	}
//}

void E64::video_t::update_screen()
{
	SDL_RenderClear(renderer);

	SDL_UpdateTexture(texture, NULL, framebuffer,
		VICV_PIXELS_PER_SCANLINE * sizeof(uint16_t));
    
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}

void E64::video_t::reset_window_size()
{
	SDL_SetWindowSize(window, window_sizes[current_window_size].x,
			  window_sizes[current_window_size].y);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
			      SDL_WINDOWPOS_CENTERED);
}

void E64::video_t::increase_window_size()
{
	if (current_window_size < 4) {
		current_window_size++;
		SDL_SetWindowSize(window, window_sizes[current_window_size].x,
				  window_sizes[current_window_size].y);
		SDL_GetWindowSize(window, &window_width, &window_height);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
				      SDL_WINDOWPOS_CENTERED);
	}
}

void E64::video_t::decrease_window_size()
{
	if (current_window_size > 0) {
		current_window_size--;
		SDL_SetWindowSize(window, window_sizes[current_window_size].x,
				  window_sizes[current_window_size].y);
		SDL_GetWindowSize(window, &window_width, &window_height);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
				      SDL_WINDOWPOS_CENTERED);
	}
}

void E64::video_t::toggle_fullscreen()
{
	fullscreen = !fullscreen;
	if (fullscreen) {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_RESIZABLE);
	}
	SDL_GetWindowSize(window, &window_width, &window_height);
}

void E64::video_t::update_title()
{
	SDL_SetWindowTitle(window, "E64");
}
