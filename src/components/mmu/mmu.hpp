/*
 * mmu.hpp
 * E64
 *
 * Copyright © 2019-2021 elmerucr. All rights reserved.
*/

#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include <cstdlib>

#define IO_VICV_PAGE		0xd0
#define IO_BLITTER_PAGE		0xd1
#define IO_TIMER_PAGE		0xd3
#define IO_SID_PAGE		0xd4
#define IO_BLITTER_PTS_PAGE	0xd8 // until 0xdfff
#define IO_ROM_PAGE		0xe0

#define IO_CIA_PAGE		0x03

namespace E64
{

class mmu_ic {
public:
	mmu_ic();
	~mmu_ic();
	
	uint8_t  *ram;          // make this private and work with friend class?
	uint8_t  current_rom_image[8192];
	
	void reset();
	
	uint8_t read_memory_8(uint16_t address);
	void write_memory_8(uint16_t address, uint8_t value);
	
	void update_rom_image();
};

}

#endif
