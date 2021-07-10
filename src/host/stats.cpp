//  stats.cpp
//  E64-SQ
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

#include <cstdint>
#include <chrono>
#include <thread>
#include <cstdint>
#include <iostream>
#include "stats.hpp"
#include "sdl2.hpp"
#include "common.hpp"


void E64::stats_t::reset()
{
	total_time = 0;
	total_idle_time = 0;
	
	framecounter = 0;
	framecounter_interval = 4;
	
	status_bar_framecounter = 0;
	status_bar_framecounter_interval = FPS / 2;

	smoothed_audio_queue_size = AUDIO_BUFFER_SIZE;
	
	smoothed_framerate = FPS;
	
	smoothed_mhz = VICV_CLOCK_SPEED/(1000*1000);
	
	smoothed_cpu_mhz = smoothed_mhz;
	old_cpu_ticks = machine.cpu->clock_ticks();
	
	smoothed_idle_per_frame = 1000000 / (FPS * 2);
    
	alpha = 0.90f;
	alpha_cpu = 0.50f;
	
	frametime = 1000000 / FPS;

	now = then = std::chrono::steady_clock::now();
}

void E64::stats_t::process_parameters()
{
	framecounter++;
	if (framecounter == framecounter_interval) {
		framecounter = 0;
        
		audio_queue_size = E64::sdl2_get_queued_audio_size();
		smoothed_audio_queue_size = (alpha * smoothed_audio_queue_size) + ((1.0 - alpha) * audio_queue_size);

		// framerate is no. of frames in one measurement interval divided by the duration
		framerate = (double)(framecounter_interval * 1000000) / total_time;
		smoothed_framerate = (alpha * smoothed_framerate) + ((1.0 - alpha) * framerate);

		mhz = (double)(framerate * (VICV_CLOCK_SPEED / FPS) )/1000000;
		smoothed_mhz = (alpha * smoothed_mhz) + ((1.0 - alpha) * mhz);
		
		new_cpu_ticks = machine.cpu->clock_ticks();
		delta_cpu_ticks = new_cpu_ticks - old_cpu_ticks;
		old_cpu_ticks = new_cpu_ticks;
		cpu_mhz = (double)delta_cpu_ticks / total_time;
		smoothed_cpu_mhz = (alpha_cpu * smoothed_cpu_mhz) + ((1.0 - alpha_cpu) * cpu_mhz);
        
		idle_per_frame = total_idle_time / (framecounter_interval);
		smoothed_idle_per_frame = (alpha * smoothed_idle_per_frame) + ((1.0 - alpha) * idle_per_frame);
        
		total_time = total_idle_time = 0;
	}

	status_bar_framecounter++;
	if (status_bar_framecounter == status_bar_framecounter_interval) {
		snprintf(statistics_string, 256, "        cpu speed:  %5.2f MHz\n   screen refresh:  %5.2f fps\n   idle per frame:  %5.2f ms\n      soundbuffer:  %5.2f kb", smoothed_cpu_mhz, smoothed_framerate, smoothed_idle_per_frame/1000, smoothed_audio_queue_size/1024);
		status_bar_framecounter = 0;
	}
}

void E64::stats_t::start_idle_time()
{
	// here we pinpoint done, because we're done with the "work"
	done = std::chrono::steady_clock::now();
}

void E64::stats_t::end_idle_time()
{
	now = std::chrono::steady_clock::now();
	total_idle_time += std::chrono::duration_cast<std::chrono::microseconds>(now - done).count();
	total_time += std::chrono::duration_cast<std::chrono::microseconds>(now - then).count();
	then = now;
}
