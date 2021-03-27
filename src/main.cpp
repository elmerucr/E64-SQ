//  main.cpp
//  E64
//
//  Copyright © 2021 elmerucr. All rights reserved.

#include <cstdio>
#include <chrono>
#include <thread>
#include "common.hpp"
#include "kernel.hpp"
#include "sdl2.hpp"

// global components
E64::host_t	host;
E64::kernel_t	*kernel;
E64::stats_t	stats;
E64::machine_t	machine;

int swallah = 0;

std::chrono::time_point<std::chrono::steady_clock> refresh_moment;

static void do_frames()
{
	if (machine.run(1023)) {
		// switch mode e.g. when have breakpoint
		//machine.switch_mode(E64::MONITOR);
	}
	
	if (machine.vicv->frame_done) {
		machine.vicv->frame_done = false;
	
		if (E64::sdl2_process_events() == E64::QUIT_EVENT) {
			machine.turned_on = false;
		}
		
		kernel->execute();
		
		machine.blitter->swap_buffers();
		machine.blitter->clear_framebuffer();
		machine.blitter->draw_blit(0, 0, 16);
		machine.blitter->draw_border();
		machine.blitter->draw_blit(1, 0, 276);
		
		machine.blitter->draw_blit(1, swallah, swallah);
		swallah++;
		if (swallah>288) swallah = -16;
		
		machine.blitter->flush();
		
		kernel->blitter->swap_buffers();
		kernel->blitter->clear_framebuffer();
		kernel->blitter->draw_blit(0, 0, 132);
		kernel->blitter->flush();
		
		host.video->clear_frame_buffer();
		host.video->merge_down_buffer(machine.blitter->frontbuffer);
		host.video->merge_down_buffer(kernel->blitter->frontbuffer);
		
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
}

int main(int argc, char **argv)
{
	E64::sdl2_init();
	machine.turned_on = true;
	machine.reset();
	
	kernel = new E64::kernel_t();
	kernel->reset();
	
	stats.reset();
	
	refresh_moment = std::chrono::steady_clock::now();

	while (machine.turned_on) {
		do_frames();
	}

	delete kernel;
	E64::sdl2_cleanup();
	return 0;
}
