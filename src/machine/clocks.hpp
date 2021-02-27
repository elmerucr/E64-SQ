//  clocks.hpp
//  E64-II
//
//  Copyright © 2019-2020 elmerucr. All rights reserved.
//
//  Algorithm based on bresenham line algorithm. Besides very simple multipliers / dividers,
//  it is also possible to build very "complex" ones.

#ifndef CLOCKS_HPP
#define CLOCKS_HPP

#include <cstdint>

namespace E64
{

class clocks
{
private:
	uint64_t clock0_frequency;
	uint64_t clock1_frequency;
	uint64_t clock0_cycles;
	uint64_t clock1_cycles;

	uint64_t mult;
	uint64_t mod;
	uint64_t result;
public:
	clocks(uint32_t clock0_freq, uint32_t clock1_freq)
	{
		clock0_frequency = clock0_freq;
		clock1_frequency = clock1_freq;
		clock0_cycles = 0;
		clock1_cycles = 0;
		mod = 0;
	}
    
	inline uint32_t clock(uint32_t delta_clock0)
	{
		mult = (delta_clock0 * clock1_frequency) + mod;
		mod  = mult % clock0_frequency;
		result = mult / clock0_frequency;
		clock0_cycles += delta_clock0;
		clock1_cycles += result;
		return (uint32_t)result;
	}
};

}

#endif
