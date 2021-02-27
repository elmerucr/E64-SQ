/*
 * vicv.cpp
 * E64-SQ
 *
 * Copyright © 2017-2021 elmerucr. All rights reserved.
 */
 
#include <cstdio>
#include "vicv.hpp"
#include "common.hpp"

E64::vicv_ic::vicv_ic()
{
	stats_visible = false;

	fb0 = (uint16_t *)&machine.mmu->ram[VICV_FB0];
	fb1 = (uint16_t *)&machine.mmu->ram[VICV_FB1];

	breakpoint_reached = false;
	clear_scanline_breakpoints();
	old_y_pos = 0;

	stats_text = nullptr;
}

void E64::vicv_ic::reset()
{
	frame_done = false;

	cycle_clock = dot_clock = 0;

	for (int i=0; i<256; i++)
		registers[i] = 0;

	frontbuffer = fb0;
	backbuffer  = fb1;
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
		bool hborder = (y_pos < registers[VICV_REG_BORDER_SIZE]) ||
			(y_pos > ((VICV_SCANLINES-1)-registers[VICV_REG_BORDER_SIZE]));

		if (!blank) {
			if (hborder) {
				host.video->framebuffer[dot_clock] =
					host.video->palette[*((uint16_t *)(&registers[VICV_REG_HOR_BOR_COL_HIGH]))];
			} else {
				host.video->framebuffer[dot_clock] =
					host.video->palette[frontbuffer[dot_clock]];
			}
			dot_clock++;	// progress dot clock if pixel was sent (!blank)
		}

		cycle_clock++;
        
		switch (cycle_clock) {
			case (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*VICV_SCANLINES:
				// start of vblank, so swap buffers
				machine.kernel->vblank_event();
				break;
			case (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*(VICV_SCANLINES+VICV_SCANLINES_VBLANK):
				// finished vblank, do other necessary stuff
				if (stats_visible) {
					render_stats(72, 276);
				}
				cycle_clock = dot_clock = 0;
				frame_done = true;
				break;
		}
	cycles--;
	}

	if ((y_pos != old_y_pos) && scanline_breakpoints[y_pos] == true)
		breakpoint_reached = true;
	old_y_pos = y_pos;
}

#define Y_POS  (cycle_clock / (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK))
#define X_POS  (cycle_clock - (Y_POS * (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)))
#define HBLANK (X_POS >= VICV_PIXELS_PER_SCANLINE)
#define VBLANK (cycle_clock>=((VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*VICV_SCANLINES))

bool E64::vicv_ic::is_hblank() { return HBLANK; }
bool E64::vicv_ic::is_vblank() { return VBLANK; }

inline void E64::vicv_ic::render_stats(uint16_t xpos, uint16_t ypos)
{
	uint32_t base = ((ypos * VICV_PIXELS_PER_SCANLINE) + xpos) %
		(VICV_PIXELS_PER_SCANLINE * VICV_SCANLINES);
	uint8_t  eight_pixels = 0;

	for (int y=0; y<8; y++) {
		char *temp_text = stats_text;
		uint16_t x = 0;
		// are we still pointing at a character
		while (*temp_text) {
			// are we at the first pixel of a char
			if (!(x & 7)) {
				eight_pixels =
					cbm_cp437_font[((*temp_text * 8) + y)];
			}

			host.video->framebuffer[base + x] = (eight_pixels & 0x80) ?
				host.video->palette[COBALT_06] :
				host.video->palette[COBALT_02];

			eight_pixels = eight_pixels << 1;
			x++;
			// increase the text pointer only when necessary
			if (!(x & 7))
				temp_text++;
		}
		// go to the next line
		base = (base + VICV_PIXELS_PER_SCANLINE) %
			(VICV_PIXELS_PER_SCANLINE * VICV_SCANLINES);
	}
}

uint16_t E64::vicv_ic::get_current_scanline() { return Y_POS; }

uint16_t E64::vicv_ic::get_current_pixel() { return X_POS; }

void E64::vicv_ic::clear_scanline_breakpoints()
{
	for (int i=0; i<1024; i++)
		scanline_breakpoints[i] = false;
}

void E64::vicv_ic::add_scanline_breakpoint(uint16_t scanline)
{
	scanline_breakpoints[scanline & 1023] = true;
}

void E64::vicv_ic::remove_scanline_breakpoint(uint16_t scanline)
{
	scanline_breakpoints[scanline & 1023] = false;
}

bool E64::vicv_ic::is_scanline_breakpoint(uint16_t scanline)
{
	return scanline_breakpoints[scanline & 1023];
}

uint8_t E64::vicv_ic::read_byte(uint8_t address)
{
	return registers[address & 0x07];
}

void E64::vicv_ic::write_byte(uint8_t address, uint8_t byte)
{
	switch (address) {
	case VICV_REG_ISR:
		if (byte & 0b00000001)
			// NEEDS WORK
			//machine.TTL74LS148->release_line(vblank_interrupt_device_number);  // acknowledge pending irq
		break;
	case VICV_REG_BUFFERSWAP:
		if (byte & 0b00000001) {
			if (machine.blitter->current_state != IDLE) {
				machine.blitter->current_state = IDLE;
				printf("[VICV] warning: blitter was not finished when swapping buffers\n");
			}
			swap_buffers();
		}
		break;
	default:
		registers[address & 0x07] = byte;
		break;
	}
}

void E64::vicv_ic::toggle_stats()
{
	stats_visible = !stats_visible;
}

void E64::vicv_ic::swap_buffers()
{
	uint16_t *tempbuffer = frontbuffer;
	frontbuffer = backbuffer;
	backbuffer = tempbuffer;
}
