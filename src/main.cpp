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

std::chrono::time_point<std::chrono::steady_clock> refresh_moment;

static void do_frames()
{
	if (machine.run(1023)) {
		// switch mode e.g. when have breakpoint
	}
	
	if (machine.vicv->frame_done()) {
		if (E64::sdl2_process_events() == E64::QUIT_EVENT) {
			machine.turned_on = false;
		}
		
		machine.blitter->swap_buffers();
		machine.blitter->clear_framebuffer();
		machine.blitter->draw_blit(0, 0, 16);
		machine.blitter->draw_border();
		machine.blitter->flush();
		
		kernel->execute();
		
		kernel->blitter->swap_buffers();
		kernel->blitter->clear_framebuffer();
		if (kernel->stats_visible || kernel->overhead_visible)
			kernel->blitter->draw_blit(kernel->stats_view->blit_no, 0, 244);
		if (kernel->overhead_visible) {
			kernel->blitter->draw_blit(kernel->terminal->blit_no, 0, 12);
			kernel->blitter->draw_blit(kernel->cpu_view->blit_no, 0, 148);
			kernel->blitter->draw_blit(kernel->stack_view->blit_no, 0, 180);
			kernel->blitter->draw_blit(kernel->disassembly_view->blit_no, 256, 148);
			kernel->blitter->draw_blit(kernel->bar_1_height->blit_no, 0, 140);
			kernel->blitter->draw_blit(kernel->bar_2_height->blit_no, 0, -4);
			kernel->blitter->draw_blit(kernel->bar_2_height->blit_no, 0, 276);
		}
		kernel->blitter->flush();
		
		host.video->clear_frame_buffer();
		
		host.video->merge_down_layer(machine.blitter->frontbuffer);
		host.video->merge_down_layer(kernel->blitter->frontbuffer);
		
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
