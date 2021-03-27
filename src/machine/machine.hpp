//  machine.hpp
//  E64
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#ifndef MACHINE_HPP
#define MACHINE_HPP

#include "cia.hpp"
#include "clocks.hpp"
#include "mmu.hpp"
#include "sids.hpp"
#include "timer.hpp"
#include "vicv.hpp"
#include "blitter.hpp"
#include "cpu.hpp"

// output states for run function
#define NO_BREAKPOINT       0b00000000
#define CPU_BREAKPOINT      0b00000001
#define SCANLINE_BREAKPOINT 0b00000010

namespace E64
{
    
class machine_t
{
private:
    clocks *vicv_to_sid;
    char machine_help_string[2048];
public:
	bool turned_on;

	mmu_ic		*mmu;
	cpu_ic		*cpu;
	timer_ic	*timer;
	vicv_ic		*vicv;
	blitter_ic	*blitter;
	sids_ic		*sids;
	cia_ic		*cia;
	//kernel_t	*kernel;

	machine_t();
	~machine_t();

	bool run(uint16_t no_of_cycles);

	void reset();
};

}

#endif
