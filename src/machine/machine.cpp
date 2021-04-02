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
	
	blitter = new blitter_ic();
	sids = new sids_ic();
	cia = new cia_ic();
	
	// init clocks (frequency dividers)
	system_to_sid   = new clocks(SYSTEM_CLOCK_SPEED, SID_CLOCK_SPEED);
	
	paused = false;
}

E64::machine_t::~machine_t()
{
	delete system_to_sid;
	
	delete cia;
	delete sids;
	delete blitter;
	delete timer;
	delete cpu;
	delete mmu;
}

bool E64::machine_t::run(uint16_t cycles)
{
	uint16_t processed_cycles = cpu->run(cycles);
	cia->run(processed_cycles);
	timer->run(processed_cycles);
	
	// run cycles on sound device & start audio if buffer is large enough
	// some cheating by adjustment of cycles to run depending on current
	// audio queue size
	unsigned int audio_queue_size = stats.current_audio_queue_size();
	
	if (audio_queue_size < 0.9 * AUDIO_BUFFER_SIZE) {
		sids->run(system_to_sid->clock(1.2 * processed_cycles));
	} else if (audio_queue_size > 1.1 * AUDIO_BUFFER_SIZE) {
		sids->run(system_to_sid->clock(0.8 * processed_cycles));
	} else {
		sids->run(system_to_sid->clock(processed_cycles));
	}
	
	if (audio_queue_size > (AUDIO_BUFFER_SIZE/2))
		E64::sdl2_start_audio();
	
	return false;
}

void E64::machine_t::reset()
{
	printf("[machine] system reset\n");
	
	mmu->reset();
	sids->reset();
	blitter->reset();
	timer->reset();
	cia->reset();
	cpu->reset();
	
	blitter->set_clear_color(C64_BLUE);
	blitter->set_border_color(C64_BLACK);
	blitter->set_border_size(16);
	
	// clear sids
	for (int i=0; i<128; i++) sids->write_byte(i, 0);
	for (int i=0; i<8; i++) sids->write_byte(128+i, 255);
	sids->write_byte(0x18, 0x0f);		// volume sid0
	sids->write_byte(0x38, 0x0f);		// volume sid1

	// sounds
	sids->write_byte(0x00, 0xc4);		// note d3
	sids->write_byte(0x01, 0x09);
	sids->write_byte(0x05, 0b00001001);	// attack/decay
	sids->write_byte(0x02, 0x0f);		// pulsewidth
	sids->write_byte(0x03, 0x0f);
	sids->write_byte(0x80, 0xff);		// sid0 left
	sids->write_byte(0x81, 0x10);		// sid0 right
	sids->write_byte(0x04, 0b01000001);	// voice control
	
	sids->write_byte(0x20, 0xa2);		// note a3
	sids->write_byte(0x21, 0x0e);
	sids->write_byte(0x25, 0b00001001);	// attack/decay
	sids->write_byte(0x22, 0x0f);		// pulsewidth
	sids->write_byte(0x23, 0x0f);
	sids->write_byte(0x82, 0x10);		// sid1 left
	sids->write_byte(0x83, 0xff);		// sid1 right
	sids->write_byte(0x24, 0b01000001);	// voice control
}
