/*
 * vicv.cpp
 * E64
 *
 * Copyright © 2017-2021 elmerucr. All rights reserved.
 */
 
#include <cstdio>
#include "vicv.hpp"
#include "common.hpp"

E64::vicv_ic::vicv_ic()
{
	frame_is_done = false;
	cycle_clock = dot_clock = 0;
}

void E64::vicv_ic::reset()
{
	registers[0] = 0;
	registers[1] = 0;
}

void E64::vicv_ic::run(uint32_t cycles)
{
	/*
	 * y_pos needs initialization otherwise compiler complains.
	 * Chosen for bogus 0xff value => probably in initialized data
	 * section. Seems to be fastest when looking at cpu usage.
	 */
	uint32_t y_pos = 0xff;
	
	uint32_t x_pos;

	while (cycles > 0) {
		y_pos = cycle_clock /
			(VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK);
		x_pos = cycle_clock -
			(y_pos * (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK));
		bool hblank = (x_pos >= VICV_PIXELS_PER_SCANLINE);
		bool vblank = cycle_clock>=((VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*VICV_SCANLINES);
		bool blank = hblank || vblank;

		if (!blank) dot_clock++;	// progress dot clock if pixel was sent (!blank)

		cycle_clock++;
        
		switch (cycle_clock) {
			case (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*VICV_SCANLINES:
				// start of vblank
				if (!machine.paused) {
					registers[0] = 0b00000001;
					machine.exceptions->pull(irq_number);
				}
				break;
			case (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*(VICV_SCANLINES+VICV_SCANLINES_VBLANK):
				// end of vblank
				cycle_clock = dot_clock = 0;
				frame_is_done = true;
				break;
		}
	cycles--;
	}
}

#define Y_POS  (cycle_clock / (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK))
#define X_POS  (cycle_clock - (Y_POS * (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)))
#define HBLANK (X_POS >= VICV_PIXELS_PER_SCANLINE)
#define VBLANK (cycle_clock>=((VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*VICV_SCANLINES))

bool E64::vicv_ic::is_hblank() { return HBLANK; }
bool E64::vicv_ic::is_vblank() { return VBLANK; }
uint16_t E64::vicv_ic::get_current_scanline() { return Y_POS; }
uint16_t E64::vicv_ic::get_current_pixel() { return X_POS; }
uint8_t E64::vicv_ic::read_byte(uint8_t address) { return registers[address & 0x01]; }

void E64::vicv_ic::write_byte(uint8_t address, uint8_t byte)
{
	switch (address) {
		case VICV_REG_ISR:
			if (byte & 0b00000001) {
				registers[0] = 0b00000000;
				machine.exceptions->release(irq_number);
			}
			break;
		default:
			registers[address & 0x01] = byte;
			break;
	}
}
