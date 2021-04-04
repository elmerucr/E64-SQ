//  main.cpp
//  E64
//
//  Copyright © 2021 elmerucr. All rights reserved.

#include <cstdio>
#include <chrono>
#include <thread>
#include "common.hpp"
#include "hud.hpp"
#include "sdl2.hpp"
#include "vicv.hpp"

#define	CYCLES_PER_STEP	511

// global components
E64::host_t	host;
E64::hud_t	hud;
E64::stats_t	stats;
E64::machine_t	machine;
E64::vicv_ic	vicv;

bool app_running;

std::chrono::time_point<std::chrono::steady_clock> refresh_moment;

static void finish_frame()
{
	if (E64::sdl2_process_events() == E64::QUIT_EVENT) app_running = false;
	
	machine.blitter->swap_buffers();
	machine.blitter->clear_framebuffer();
	machine.blitter->draw_blit(0, 0, 16);
	machine.blitter->draw_border();
	machine.blitter->flush();
	
	if (hud.refreshed()) {
		hud.blitter->swap_buffers();
		hud.blitter->clear_framebuffer();
		hud.redraw();
		hud.blitter->flush();
	}
	
	host.video->clear_frame_buffer();
	host.video->merge_down_layer(machine.blitter->frontbuffer);
	host.video->merge_down_layer(hud.blitter->frontbuffer);
	
	stats.process_parameters();
	/*
		* If vsync is enabled, the update screen function takes more
		* time, i.e. it will return after a few milliseconds, exactly
		* when vertical refresh is done. This will avoid tearing.
		* Moreover, there's no need then to let the system sleep with a
		* calculated value. But we will still have to do a time
		* measurement for estimation of idle time.
		*/
	stats.start_idle_time();
	if (host.video->vsync_disabled()) {
		refresh_moment +=
		std::chrono::microseconds(stats.frametime);
		/*
			* Check if the next update is in the past,
			* this can be the result of a debug session.
			* If so, calculate a new update moment. This will
			* avoid "playing catch-up" by the virtual machine.
			*/
		if (refresh_moment < std::chrono::steady_clock::now())
			refresh_moment = std::chrono::steady_clock::now() +
			std::chrono::microseconds(stats.frametime);
		std::this_thread::sleep_until(refresh_moment);
	}
	host.video->update_screen();
	stats.end_idle_time();
}

int main(int argc, char **argv)
{
	E64::sdl2_init();
	
	app_running = true;
	
	vicv.reset();
	hud.reset();
	machine.reset();
	stats.reset();
	
	// if one is paused, the other shouldn't be
	machine.paused = false;
	hud.paused = true;
	
	refresh_moment = std::chrono::steady_clock::now();

	while (app_running) {
		vicv.run(CYCLES_PER_STEP);
		
		if (!hud.paused) {
			hud.run(CYCLES_PER_STEP);
		}
		
		if (!machine.paused) {
			if (machine.run(CYCLES_PER_STEP)) {
				// switch mode e.g. when have breakpoint
			}
		}
		
		if (vicv.frame_done())
			finish_frame();
	}

	E64::sdl2_cleanup();
	return 0;
}
