#include "kernel.hpp"
#include "common.hpp"

E64::kernel_t::kernel_t()
{
	heap_start = &machine.mmu->ram[0x000000];
	heap_end   = &machine.mmu->ram[0xf00000];
}

void E64::kernel_t::reset()
{
	machine.vicv->registers[VICV_REG_BORDER_SIZE] = 16;
	
	// startup sound
	for (int i=0; i<128; i++) {
		machine.sids->write_byte(i, 0);
	}
	for (int i=0; i<8; i++) {
		machine.sids->write_byte(128+i, 255);
	}
	
	machine.sids->write_byte(0x1b, 0x0f);		// volume sid0
	machine.sids->write_byte(0x3b, 0x0f);		// volume sid1

	machine.sids->write_byte(0x00, 0x09);		// note d3
	machine.sids->write_byte(0x01, 0xc4);
	machine.sids->write_byte(0x05, 0b00001001);	// attack/decay
	machine.sids->write_byte(0x02, 0x0f);		// pulsewidth
	machine.sids->write_byte(0x03, 0x0f);
	machine.sids->write_byte(0x80, 0xff);		// sid0 left
	machine.sids->write_byte(0x81, 0x10);		// sid0 right
	machine.sids->write_byte(0x04, 0b01000001);	// voice control
	
	machine.sids->write_byte(0x20, 0x0e);		// note a3
	machine.sids->write_byte(0x21, 0xa2);
	machine.sids->write_byte(0x25, 0b00001001);	// attack/decay
	machine.sids->write_byte(0x22, 0x0f);		// pulsewidth
	machine.sids->write_byte(0x23, 0x0f);
	machine.sids->write_byte(0x82, 0x10);		// sid0 left
	machine.sids->write_byte(0x83, 0xff);		// sid0 right
	machine.sids->write_byte(0x24, 0b01000001);	// voice control
	
}

void E64::kernel_t::run()
{
	//
}

void E64::kernel_t::vblank_event()
{
	machine.vicv->swap_buffers();
}

void E64::kernel_t::timer_event()
{
	//
}
