//  machine.cpp
//  E64
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#include "machine.hpp"
#include "sdl2.hpp"
#include "common.hpp"

E64::machine_t::machine_t()
{
	mmu = new mmu_ic();
	cpu = new cpu_ic();
	timer = new timer_ic();
	
	vicv = new vicv_ic();
	
	blitter = new blitter_ic();
	sids = new sids_ic();
	cia = new cia_ic();
	
	//kernel = new kernel_t();
	
	// init clocks (frequency dividers, right no of cycles will run on different ic's)
	vicv_to_sid   = new clocks(VICV_DOT_CLOCK_SPEED, SID_CLOCK_SPEED );
}

E64::machine_t::~machine_t()
{
	delete vicv_to_sid;
	
	//delete kernel;
	delete cia;
	delete sids;
	delete blitter;
	delete vicv;
	delete timer;
	delete cpu;
	delete mmu;
}

bool E64::machine_t::run(uint16_t cycles)
{
	vicv->run(cycles);
	timer->run(cycles);
	cia->run(cycles);
	
	// run cycles on sound device & start audio if buffer is large enough
	// some cheating by adjustment of cycles to run depending on current
	// audio queue size
	unsigned int audio_queue_size = stats.current_audio_queue_size();
	
	if (audio_queue_size < 0.9 * AUDIO_BUFFER_SIZE) {
		sids->run(vicv_to_sid->clock(1.2 * cycles));
	} else if (audio_queue_size > 1.1 * AUDIO_BUFFER_SIZE) {
		sids->run(vicv_to_sid->clock(0.8 * cycles));
	} else {
		sids->run(vicv_to_sid->clock(cycles));
	}
	
	if (audio_queue_size > (AUDIO_BUFFER_SIZE/2))
		E64::sdl2_start_audio();
	
	return false;
}

void E64::machine_t::reset()
{
	sids->reset();
	vicv->reset();
	blitter->reset();
	timer->reset();
	cia->reset();
	cpu->reset();
	printf("[machine] system reset\n");
}
