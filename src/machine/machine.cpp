//  machine.cpp
//  E64-II
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#include "machine.hpp"
#include "sdl2.hpp"
#include "common.hpp"

E64::machine_t::machine_t()
{
	mmu = new mmu_ic();
	timer = new timer_ic();
	
	vicv = new vicv_ic();
	vicv->set_stats(stats.summary());
	
	blitter = new blitter_ic();
	sids = new sids_ic();
	cia = new cia_ic();
	
	kernel = new kernel_t();
	
	// init clocks (frequency dividers, right no of cycles will run on different ic's)
	vicv_to_blitter = new clocks(VICV_DOT_CLOCK_SPEED, BLITTER_DOT_CLOCK_SPEED);
	vicv_to_sid   = new clocks(VICV_DOT_CLOCK_SPEED, SID_CLOCK_SPEED );
}

E64::machine_t::~machine_t()
{
	delete vicv_to_sid;
	delete vicv_to_blitter;
	
	delete kernel;
	delete cia;
	delete sids;
	delete blitter;
	delete vicv;
	delete timer;
	delete mmu;
}

/*
 * Note: using run(0) function causes the cpu to run only 1 instruction per
 * call. This will increase the overall host cpu load, but also increases
 * accuracy of the system as a whole. Most importantly, SID and VICV emulation
 * will be very realistic. Instant changes to registers should be reflected in
 * audio output.
 * However, run(63) significantly reduces host cpu load, once we have some music
 * running in the virtual machine, test this.
 */
void E64::machine_t::run(uint16_t cycles)
{
	// run cycles on vicv and check for breakpoints
	vicv->run(cycles);
	if (vicv->breakpoint_reached) {
		snprintf(machine_help_string, 256,
			 "scanline breakpoint occurred at line %i\n",
			 vicv->get_current_scanline());
		vicv->breakpoint_reached = false;
	}
    
	blitter->run(vicv_to_blitter->clock(cycles));
	timer->run(cycles);
	cia->run(cycles);
	
	// run cycles on sound device & start audio if buffer is large enough
	// some cheating by adjustment of cycles to run depending on current
	// audio queue size
	unsigned int audio_queue_size = stats.current_audio_queue_size();
	
	if (audio_queue_size < 0.9 * AUDIO_BUFFER_SIZE)
		sids->run(vicv_to_sid->clock(1.2 * cycles));
	else if (audio_queue_size > 1.1 * AUDIO_BUFFER_SIZE)
		sids->run(vicv_to_sid->clock(0.8 * cycles));
	else
		sids->run(vicv_to_sid->clock(cycles));
	
	if (audio_queue_size > (AUDIO_BUFFER_SIZE/2))
		E64::sdl2_start_audio();
}

void E64::machine_t::reset()
{
	host.video->reset();
	sids->reset();
	vicv->reset();
	blitter->reset();    // sometimes warning message blitter not finished
	timer->reset();
	cia->reset();
	kernel->reset();
	printf("[machine] system reset\n");
}
