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
bool		app_running;
std::chrono::time_point<std::chrono::steady_clock> refresh_moment;

static void finish_frame();

int main(int argc, char **argv)
{
	E64::sdl2_init();
	
	app_running = true;
	
	vicv.reset();
	vicv.irq_number = machine.exceptions->connect_device();
	
	hud.reset();
	machine.reset();
	stats.reset();
	
	// if one is paused, the other shouldn't be
	machine.paused = false;
	hud.paused = true;
	
	refresh_moment = std::chrono::steady_clock::now();

	while (app_running) {
		vicv.run(CYCLES_PER_STEP);
		
		if (machine.paused) {
			hud.run(CYCLES_PER_STEP);
		} else {
			if (machine.run(CYCLES_PER_STEP)) {
				// ugly, needs better way...
				hud.flip_modes();
				hud.terminal->printf("breakpoint reached at $%04x\n",
						     machine.cpu->get_pc());
			}
		}
		
		if (vicv.frame_done())
			finish_frame();
	}

	E64::sdl2_cleanup();
	return 0;
}

static void finish_frame()
{
	if (E64::sdl2_process_events() == E64::QUIT_EVENT) app_running = false;
	
	machine.blit->flush();
	
	if (!hud.paused) {
		hud.process_keypress();
		hud.update();
	}
	
	hud.update_stats_view();
	hud.blit->swap_buffers();
	hud.blit->clear_framebuffer();
	hud.redraw();
	hud.blit->flush();
	
	host.video->clear_frame_buffer();
	host.video->merge_down_layer(machine.blit->frontbuffer);
	host.video->merge_down_layer(hud.blit->frontbuffer);
	
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
