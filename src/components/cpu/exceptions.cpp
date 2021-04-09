//  exceptions.cpp
//  E64
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#include "exceptions.hpp"

E64::exceptions_ic::exceptions_ic()
{
    for(int i=0; i<8; i++) irq_input_pins[i] = true;
    next_available_device = 0;
    nmi_output_pin = true;
	update_status();
}

uint8_t E64::exceptions_ic::connect_device()
{
	uint8_t return_value = next_available_device;
	next_available_device++;
	next_available_device &= 7;
	return return_value;
}

void E64::exceptions_ic::update_status()
{
	bool result =	(!(irq_input_pins[0])) |
			(!(irq_input_pins[1])) |
			(!(irq_input_pins[2])) |
			(!(irq_input_pins[3])) |
			(!(irq_input_pins[4])) |
			(!(irq_input_pins[5])) |
			(!(irq_input_pins[6])) |
			(!(irq_input_pins[7])) ;
	irq_output_pin = !result;
}

void E64::exceptions_ic::pull(uint8_t device)
{
	irq_input_pins[device & 0x7] = false;
	update_status();
}

void E64::exceptions_ic::release(uint8_t device)
{
	irq_input_pins[device & 0x7] = true;
	update_status();
}
