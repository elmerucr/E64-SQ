#include "kernel.hpp"
#include "common.hpp"

E64::kernel_t::kernel_t()
{
	memory_manager = new allocation(&machine.mmu->ram[0x000000], &machine.mmu->ram[0xf00000]);
	v = sq_open(1024); // creates a squirrel VM with initial stack size 1024
	
	void *test = memory_manager->malloc(31);
	
	printf("address of *test: %016x\n", test);
	
	memory_manager->realloc(test, 677);
	
	memory_manager->free(test);
}

E64::kernel_t::~kernel_t()
{
	sq_close(v);
	delete memory_manager;
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
	
	machine.sids->write_byte(0x18, 0x0f);		// volume sid0
	machine.sids->write_byte(0x38, 0x0f);		// volume sid1

	machine.sids->write_byte(0x00, 0xc4);		// note d3
	machine.sids->write_byte(0x01, 0x09);
	machine.sids->write_byte(0x05, 0b00001001);	// attack/decay
	machine.sids->write_byte(0x02, 0x0f);		// pulsewidth
	machine.sids->write_byte(0x03, 0x0f);
	machine.sids->write_byte(0x80, 0xff);		// sid0 left
	machine.sids->write_byte(0x81, 0x10);		// sid0 right
	machine.sids->write_byte(0x04, 0b01000001);	// voice control
	
	machine.sids->write_byte(0x20, 0xa2);		// note a3
	machine.sids->write_byte(0x21, 0x0e);
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
