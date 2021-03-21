#include "kernel.hpp"
#include "common.hpp"

E64::kernel_t::kernel_t()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_math(L);
	luaopen_string(L);
	
	tty = new tty_t(0b10001000, 0b00110000, 0x56, 256, COBALT_06, COBALT_02);
	message_box = new tty_t(0b10001010, 0b00000100, 0x05, 257, GREEN_06, (GREEN_02 & 0x0fff) | 0xa000);
	message_box->clear();
	message_box->printf("  Here we have a piece of info");
}

E64::kernel_t::~kernel_t()
{
	delete message_box;
	delete tty;
	lua_close(L);
}

void E64::kernel_t::reset()
{
	machine.vicv->set_horizontal_border_color(C64_BLACK);
	machine.vicv->set_horizontal_border_size(16);
	machine.blitter->set_clearcolor(COBALT_02);
	
	// clear sids
	for (int i=0; i<128; i++) machine.sids->write_byte(i, 0);
	for (int i=0; i<8; i++) machine.sids->write_byte(128+i, 255);
	machine.sids->write_byte(0x18, 0x0f);		// volume sid0
	machine.sids->write_byte(0x38, 0x0f);		// volume sid1

	// sounds
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
	
	devices.cia_set_keyboard_repeat_delay(50);
	devices.cia_set_keyboard_repeat_speed(5);
	devices.cia_generate_key_events();
	
	devices.timer_set(0, 3600);
	
	tty->clear();
	tty->printf("E64 Virtual Computer System (C)%u elmerucr\n\n", E64_SQ_YEAR);
	tty->puts(LUA_COPYRIGHT);
	tty->putchar('\n');
	tty->prompt();
	tty->activate_cursor();
}

void E64::kernel_t::process_keypress()
{
	uint8_t key_value = machine.cia->read_byte(0x04);
	switch (key_value) {
		case ASCII_CURSOR_LEFT:
			tty->cursor_left();
			break;
		case ASCII_CURSOR_RIGHT:
			tty->cursor_right();
			break;
		case ASCII_CURSOR_UP:
			tty->cursor_up();
			break;
		case ASCII_CURSOR_DOWN:
			tty->cursor_down();
			break;
		case ASCII_BACKSPACE:
			tty->backspace();
			break;
		case ASCII_LF:
		{
			char *buffer = tty->enter_command();
			tty->putchar('\n');
			if (*buffer) {
				tty->puts(buffer);
				tty->prompt();
			}
		}
			break;
		default:
			tty->putchar(key_value);
			break;
	}
}

void E64::kernel_t::execute()
{
	while (machine.cia->read_byte(0x00)) {
		tty->deactivate_cursor();
		process_keypress();
		tty->activate_cursor();
	}
}

void E64::kernel_t::vblank_event()
{
	machine.vicv->swap_buffers();
	machine.blitter->add_clear_framebuffer();
	//machine.blitter->add_blit(tty->text_screen, 0, 16);
	machine.blitter->add_blit(256, 0, 16);
	//machine.blitter->add_blit(message_box->text_screen, 100, 100);
	machine.blitter->add_blit(257, 100, 100);
}

void E64::kernel_t::timer_0_event()
{
	tty->timer_callback();
}

void E64::kernel_t::timer_1_event()
{
	//
}

void E64::kernel_t::timer_2_event()
{
	//
}

void E64::kernel_t::timer_3_event()
{
	//
}

void E64::kernel_t::timer_4_event()
{
	//
}

void E64::kernel_t::timer_5_event()
{
	//
}

void E64::kernel_t::timer_6_event()
{
	//
}

void E64::kernel_t::timer_7_event()
{
	//
}

//void E64::kernel_t::build_character_ram()
//{
//	cbm_font = (uint16_t *)machine.mmu->malloc(64 * 256 * sizeof(uint16_t));
//
//	uint16_t *dest = cbm_font;
//
//	for (int i=0; i<2048; i++) {
//		uint8_t byte = cbm_cp437_font[i];
//		uint8_t count = 8;
//		while (count--) {
//			*dest = (byte & 0b10000000) ? C64_GREY : 0x0000;
//			dest++;
//			byte = byte << 1;
//		}
//	}
//}
