//  main.cpp
//  E64-SQ
//
//  Copyright © 2021 elmerucr. All rights reserved.

#include <cstdio>
#include <chrono>
#include <thread>
#include "common.hpp"
#include "sdl2.hpp"

// global components
E64::host_t	host;
E64::stats_t	stats;
E64::machine_t	machine;

std::chrono::time_point<std::chrono::steady_clock> refresh_moment;

static void do_frame()
{
	for (;;) {
		machine.run(63);
		if (machine.vicv->frame_done) {
			break;
		}
	}
	machine.vicv->frame_done = false;
	if (E64::sdl2_process_events() == E64::QUIT_EVENT) {
		machine.turned_on = false;
	}
	machine.kernel->execute();
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

	machine.turned_on = true;

	stats.reset();
	
	refresh_moment = std::chrono::steady_clock::now();

	while (machine.turned_on) {
		do_frame();
	}

	E64::sdl2_cleanup();
	return 0;
}
