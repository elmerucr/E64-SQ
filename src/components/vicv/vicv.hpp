/*
 * vicv.hpp
 * E64
 *
 * Copyright © 2017-2021 elmerucr. All rights reserved.
 */

#include <cstdint>

#ifndef VICV_HPP
#define VICV_HPP

/*
 * Register 0x00 is interrupt status register. Write to bit 0 means
 * acknowledge VBLANK interrupt.
 */
#define VICV_REG_ISR		0x00

namespace E64 {

class vicv_ic
{
private:
	uint32_t cycle_clock;	// measures all cycles
	uint32_t dot_clock;	// measures only cycles that wrote a pixel
	
	// this will be flagged if a frame is completely done
	bool frame_is_done;
public:
	uint8_t registers[2];
	
	void reset();
	
	inline bool frame_done() {
		bool result = frame_is_done;
		if (frame_is_done)
			frame_is_done = false;
		return result;
	}

	// run cycles on this chip
	void run(uint32_t cycles);

	uint16_t        get_current_scanline();
	uint16_t        get_current_pixel();
	bool            is_hblank();
	bool            is_vblank();

	// Register access to vicv
	uint8_t read_byte(uint8_t address);
	void    write_byte(uint8_t address, uint8_t byte);
};

}

#endif
