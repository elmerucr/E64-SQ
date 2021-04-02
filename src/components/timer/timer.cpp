//  timer.cpp
//  E64
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#include "timer.hpp"
#include "common.hpp"

void E64::timer_ic::reset()
{
	registers[0] = 0x00;	// no pending irq's
	registers[1] = 0x00;	// all timers turned off
    
	// load data register with value 1 bpm (may never be zero)
	registers[2] = 0x01;	// low byte
	registers[3] = 0x00;	// high byte
	
	for (int i=0; i<8; i++) {
		timers[i].bpm = registers[2] | (registers[3] << 8);
		timers[i].clock_interval = bpm_to_clock_interval(timers[i].bpm);
		timers[i].counter = 0;
	}
	
	irq_line = true;
}

void E64::timer_ic::run(uint32_t number_of_cycles)
{
	for (int i=0; i<8; i++) {
		timers[i].counter += number_of_cycles;
		if ((timers[i].counter >= timers[i].clock_interval) &&
		    (registers[1] & (0b1 << i))) {
			timers[i].counter -= timers[i].clock_interval;
			irq_line = false;
			registers[0] |= (0b1 << i);
		}
	}
}

uint32_t E64::timer_ic::bpm_to_clock_interval(uint16_t bpm)
{
	return (60.0 / bpm) * VICV_DOT_CLOCK_SPEED;
}

uint8_t E64::timer_ic::read_byte(uint8_t address)
{
	return registers[address & 0x03];
}

void E64::timer_ic::write_byte(uint8_t address, uint8_t byte)
{
	switch (address & 0x03) {
	case 0x00:
            /*
             *  b s   r
             *  0 0 = 0
             *  0 1 = 1
             *  1 0 = 0
             *  1 1 = 0
             *
             *  b = bit that's written
             *  s = status (on if an interrupt was caused)
             *  r = boolean result (acknowledge an interrupt (s=1) if b=1
             *  r = (~b) & s
	     */

            registers[0] = (~byte) & registers[0];

            if ((registers[0] & 0xff) == 0) {
                // no timers left causing interrupts
		    // NEEDS WORK
                //machine.TTL74LS148->release_line(interrupt_device_number);
		    irq_line = true;
            }
            break;
        case 0x01:
	{
		uint8_t turned_on = byte & (~registers[1]);
		
		for (int i=0; i<8; i++) {
			if (turned_on & (0b1 << i)) {
				timers[i].bpm = (uint16_t)registers[2] | (registers[3] << 8);
				if (timers[i].bpm == 0)
					timers[i].bpm = 1;
				timers[i].clock_interval =
					bpm_to_clock_interval(timers[i].bpm);
				timers[i].counter = 0;
			}
		}
		registers[0x01] = byte; //& 0x0f;	// turn off all the rest?
		break;
	}
	default:
		registers[ address & 0x03 ] = byte;
		break;
	}
}

uint64_t E64::timer_ic::get_timer_counter(uint8_t timer_number)
{
	return timers[timer_number & 0x07].counter;
}

uint64_t E64::timer_ic::get_timer_clock_interval(uint8_t timer_number)
{
	return timers[timer_number & 0x07].clock_interval;
}

void E64::timer_ic::set(uint8_t timer_no, uint16_t bpm)
{
	timer_no &= 0b111; // confine to 0 - 7
	
	write_byte(0x02, bpm & 0xff);
	write_byte(0x03, (bpm & 0xff00) >> 8);
	write_byte(0x01, 0b1 << timer_no);
}
