//  exceptions.hpp
//  E64
//
//  Copyright © 2019-2021 elmerucr. All rights reserved.

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <cstdint>

namespace E64
{

class exceptions_ic {
private:
	uint8_t next_available_device;
	void update_status();
public:
	bool irq_input_pins[8];
	exceptions_ic();
	bool irq_output_pin;
	bool nmi_output_pin;
	uint8_t connect_device();
	void pull(uint8_t device);
	void release(uint8_t device);
};

}

#endif
