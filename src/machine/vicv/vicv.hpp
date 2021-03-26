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

/*
 * Register 0x01, write to bit 0 = swap front and back buffer
 * (machine internally).
 */
#define VICV_REG_BUFFERSWAP	0x01

namespace E64 {

class vicv_ic
{
public:
	// this chip contains 256 registers (and are mapped to a specific page)
	uint8_t registers[256];
private:
	// framebuffer pointers
	uint16_t *fb0;
	uint16_t *fb1;

	uint32_t cycle_clock;	// measures all cycles
	uint32_t dot_clock;	// measures only cycles that wrote a pixel

	bool scanline_breakpoints[1024];
	uint16_t old_y_pos;
public:
	bool breakpoint_reached;

public:
	void swap_buffers();
    
public:
	vicv_ic();
	~vicv_ic();

	// framebuffer pointers inside the virtual machine
	uint16_t *frontbuffer;
	uint16_t *backbuffer;

	// this will be flagged if a frame is completely done
	bool frame_done;

	void reset();

	// run cycles on this chip
	void run(uint32_t cycles);

	uint16_t        get_current_scanline();
	uint16_t        get_current_pixel();
	bool            is_hblank();
	bool            is_vblank();
    
	void clear_scanline_breakpoints();
	void add_scanline_breakpoint(uint16_t scanline);
	void remove_scanline_breakpoint(uint16_t scanline);
	bool is_scanline_breakpoint(uint16_t scanline);

	// Register access to vicv
	uint8_t read_byte(uint8_t address);
	void write_byte(uint8_t address, uint8_t byte);
};

}

#endif
