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

#define IO_VICV			0xd0 // until 0xd0ff

#define IO_BLIT			0xd1 // until 0xd1ff
#define IO_BLIT_MEMORY		0xd2 // until 0xd2ff
#define IO_BLIT_DESCRIPTOR	0xd8 // until 0xdfff

#define IO_TIMER_PAGE		0xd3 // until 0xd3ff

#define IO_SID_PAGE		0xd4

#define IO_CIA_PAGE		0xd5

#define IO_ROM_PAGE		0xe0

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
