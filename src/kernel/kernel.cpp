#include "kernel.hpp"
#include "common.hpp"

E64::kernel_t::kernel_t()
{
	v = sq_open(1024); // creates a squirrel VM with initial stack size 1024
	init_chars();
	tty = new tty_t(0x56, chars, C64_LIGHTBLUE, C64_BLUE);
}

E64::kernel_t::~kernel_t()
{
	delete tty;
	sq_close(v);
}

void E64::kernel_t::reset()
{
	machine.vicv->set_horizontal_border_color(C64_BLACK);
	machine.vicv->set_horizontal_border_size(16);
	machine.blitter->set_clearcolor(C64_BLUE);
	
	probeersel = (surface_t *)machine.mmu->malloc(sizeof(surface_t));
	
	probeersel->flags_0 = 0x05;
	probeersel->flags_1 = 0x05;
	probeersel->foreground_color = C64_LIGHTBLUE;
	probeersel->background_color = C64_RED;
	probeersel->size_in_tiles_log2 = 0x00;
	probeersel->pixel_data = (uint16_t *)machine.mmu->malloc(64);
	probeersel->pixel_data = &chars[64*64];
	probeersel->pixel_data[0] = C64_BLACK;
	probeersel->pixel_data[9] = C64_BLACK;
	probeersel->pixel_data[18] = C64_BLACK;
	probeersel->pixel_data[63] = C64_GREEN;
	
	xje = -16;
	
	tty->puts("elmer\n");
	tty->printf("$%08x\tis een test\n", 0xdeadbeef);
	
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
	machine.sids->write_byte(0x82, 0x10);		// sid1 left
	machine.sids->write_byte(0x83, 0xff);		// sid1 right
	machine.sids->write_byte(0x24, 0b01000001);	// voice control
}

void E64::kernel_t::execute()
{
	//
}

void E64::kernel_t::vblank_event()
{
	machine.vicv->swap_buffers();
	machine.blitter->clear_framebuffer();
	machine.blitter->add_blit(probeersel, xje, xje);
	xje++;
	if (xje > 288) xje = -16;
	
	machine.blitter->add_blit(tty->text_screen, 0, 16);
	tty->putsymbol(xje & 0xff);
}

void E64::kernel_t::timer_event()
{
	//
}

void E64::kernel_t::init_chars()
{
	chars = (uint16_t *)machine.mmu->malloc(64 * 256 * sizeof(uint16_t));
	
	uint16_t *dest = chars;
	
	for (int i=0; i<2048; i++) {
		uint8_t byte = cbm_cp437_font[i];
		uint8_t count = 8;
		while (count--) {
			*dest = (byte & 0b10000000) ? C64_GREY : 0x0000;
			dest++;
			byte = byte << 1;
		}
	}
}
