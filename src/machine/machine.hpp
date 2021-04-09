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
#include "blitter.hpp"
#include "cpu.hpp"
#include "exceptions.hpp"

namespace E64
{

class machine_t {
private:
	clocks *system_to_sid;
	char machine_help_string[2048];
public:
	bool paused;

	mmu_ic		*mmu;
	exceptions_ic	*exceptions;
	cpu_ic		*cpu;
	timer_ic	*timer;
	blitter_ic	*blitter;
	sids_ic		*sids;
	cia_ic		*cia;

	machine_t();
	~machine_t();

	bool run(uint16_t no_of_cycles);

	void reset();
};

}

#endif
