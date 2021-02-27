//  timer.hpp
//  E64-II
//
//  Copyright © 2019-2020 elmerucr. All rights reserved.

/*
 * Register 0 - Status register
 *
 * (READ)
 * 7 6 5 4 3 2 1 0
 *         | | | |
 *         | | | +-- 1 = timer0 pulled the irq
 *         | | +---- 1 = timer1 pulled the irq
 *         | +------ 1 = timer2 pulled the irq
 *         +-------- 1 = timer3 pulled the irq
 *
 * bits 4-7: Reserved
 *
 * On write, interrupts will be acknowledged. Only if all bits are zero again,
 * interrupt line is up again.
 */

/*
 * Register 1 - Control Register
 *
 * READ and WRITE:
 * 7 6 5 4 3 2 1 0
 *         | | | |
 *         | | | +-- timer0 interrupts, 1=on, 0=off
 *         | | +---- timer1 interrupts, 1=on, 0=off
 *         | +------ timer2 interrupts, 1=on, 0=off
 *         +-------- timer3 interrupts, 1=on, 0=off
 *
 * bits 4-7: Reserved
 */
 
/*
 * register 2 and 3 are respectively the high and low byte
 * of an unsigned 16bit data value
 */

#ifndef timer_hpp
#define timer_hpp

#include <cstdint>

namespace E64
{

struct timer_unit {
	uint16_t bpm;
	uint32_t clock_interval;
	uint64_t counter;
};

class timer_ic
{
private:
	uint8_t registers[4];
	
	struct timer_unit timers[8];

	uint32_t bpm_to_clock_interval(uint16_t bpm);
public:
	void reset();

	// register access functions
	uint8_t read_byte(uint8_t address);
	void write_byte(uint8_t address, uint8_t byte);

	// get functions
	uint64_t get_timer_counter(uint8_t timer_number);
	uint64_t get_timer_clock_interval(uint8_t timer_number);

	// run cycles on this ic
	void run(uint32_t number_of_cycles);
};

}

#endif
