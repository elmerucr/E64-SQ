#include "kernel.hpp"
#include "common.hpp"

uint16_t overhead_positions[30] = {
	0 ,1 ,2, 5, 8, 11, 15, 20, 25, 32, 38, 46, 54, 62, 72, 81, 92,
	103, 115, 128, 141, 154, 169, 184, 200, 216, 233, 250, 269, 288
};

E64::kernel_t::kernel_t()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_math(L);
	luaopen_string(L);
	
	blitter = new blitter_ic();
	cia = new cia_ic();
	
	//tty = new tty_t(0b10001000, 0b00000000, 0x56, 0, machine.blitter, C64_LIGHTBLUE, C64_BLUE);
	
	stats_view = new tty_t(0b10001010, 0b00000000, 0x25, 0, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	terminal = new tty_t(0b10001010, 0b00000000, 0x46, 1, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	cpu_view = new tty_t(0b10001010, 0b00000000, 0x25, 2, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	disassembly_view = new tty_t(0b10001010, 0b00000000, 0x45, 3, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	stack_view = new tty_t(0b10001010, 0b00000000, 0x35, 4, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	bar_1_height = new tty_t(0b00001111, 0b00000000, 0x06, 5, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	bar_2_height = new tty_t(0b10001010, 0b00000000, 0x16, 6, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	
	stats_visible = false;
	overhead_visible = false;
	overhead_state = OVERHEAD_NOT_VISIBLE;
	
	machine.mmu->write_memory_8(0x1010, 0x0f);
	machine.mmu->write_memory_8(0x1011, 0x53);
	machine.mmu->write_memory_8(0x1012, 0x01);
	machine.mmu->write_memory_8(0x1013, 0xc0);
}

E64::kernel_t::~kernel_t()
{
	delete bar_2_height;
	delete bar_1_height;
	delete stack_view;
	delete disassembly_view;
	delete cpu_view;
	delete terminal;
	delete stats_view;
	//delete tty;
	
	delete cia;
	delete blitter;
	
	lua_close(L);
}

void E64::kernel_t::reset()
{
	
	machine.blitter->set_clear_color(C64_BLUE);
	machine.blitter->set_border_color(C64_BLACK);
	machine.blitter->set_border_size(16);
	
	blitter->reset();
	blitter->set_clear_color(0x0000);
	blitter->set_border_color(0x0000);
	blitter->set_border_size(0);
	
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
	
	terminal->clear();
	terminal->printf("E64 Virtual Computer System (C)%u elmerucr\n\n", E64_SQ_YEAR);
	terminal->puts(LUA_COPYRIGHT);
	terminal->putchar('\n');
	terminal->prompt();
	terminal->activate_cursor();
	
	stack_view->clear();
	bar_1_height->clear();
	for (int i = 1536; i<2048; i++)
		blitter->blit[bar_1_height->blit_no].pixel_data[i] = GREEN_05;
	
	bar_2_height->clear();
}

void E64::kernel_t::process_keypress()
{
	uint8_t key_value = machine.cia->read_byte(0x04);
	switch (key_value) {
		case ASCII_CURSOR_LEFT:
			terminal->cursor_left();
			break;
		case ASCII_CURSOR_RIGHT:
			terminal->cursor_right();
			break;
		case ASCII_CURSOR_UP:
			terminal->cursor_up();
			break;
		case ASCII_CURSOR_DOWN:
			terminal->cursor_down();
			break;
		case ASCII_BACKSPACE:
			terminal->backspace();
			break;
		case ASCII_LF:
			{
				char *buffer = terminal->enter_command();
				terminal->putchar('\n');
				machine.cpu->run(0);
				if (*buffer) {
					terminal->puts(buffer);
					terminal->prompt();
				}
			}
			break;
		default:
			terminal->putchar(key_value);
			break;
	}
}

void E64::kernel_t::execute()
{
	while (machine.cia->read_byte(0x00)) {
		terminal->deactivate_cursor();
		process_keypress();
		terminal->activate_cursor();
	}
	
	stats_view->clear();
	stats_view->puts(stats.summary());
	
	cpu_view->clear();
	cpu_view->printf("  pc  ac xr yr sp nv-bdizc I Nn\n");
	cpu_view->printf(" %04x %02x %02x %02x %02x %c%c%c%c%c%c%c%c %c %c%c",
			 machine.cpu->get_pc(),
			 machine.cpu->get_a(),
			 machine.cpu->get_x(),
			 machine.cpu->get_y(),
			 machine.cpu->get_sp(),
			 machine.cpu->get_status() & 0x80 ? '*' : '.',
			 machine.cpu->get_status() & 0x40 ? '*' : '.',
			 machine.cpu->get_status() & 0x20 ? '*' : '.',
			 machine.cpu->get_status() & 0x10 ? '*' : '.',
			 machine.cpu->get_status() & 0x08 ? '*' : '.',
			 machine.cpu->get_status() & 0x04 ? '*' : '.',
			 machine.cpu->get_status() & 0x02 ? '*' : '.',
			 machine.cpu->get_status() & 0x01 ? '*' : '.',
			 machine.cpu->get_irq_line() ? '1' : '0',
			 machine.cpu->get_nmi_line() ? '1' : '0',
			 machine.cpu->get_old_nmi_line() ? '1' : '0');
	
	disassembly_view->clear();
	char text_buffer[256];
	uint16_t pc = machine.cpu->get_pc();
	for (int i=0; i<16; i++) {
		if (disassembly_view->get_column() != 0)
			disassembly_view->putchar('\n');
		int ops = machine.cpu->disassemble(pc, text_buffer);
		switch (ops) {
		case 1:
			disassembly_view->printf(" %04x %02x       %s",
						 pc,
						 machine.mmu->read_memory_8(pc),
						 text_buffer);
			break;
		case 2:
			disassembly_view->printf(" %04x %02x %02x    %s",
						 pc,
						 machine.mmu->read_memory_8(pc),
						 machine.mmu->read_memory_8(pc+1),
						 text_buffer);
			break;
		case 3:
			disassembly_view->printf(" %04x %02x %02x %02x %s",
						 pc,
						 machine.mmu->read_memory_8(pc),
						 machine.mmu->read_memory_8(pc+1),
						 machine.mmu->read_memory_8(pc+2),
						 text_buffer);
			break;
		}
		pc += ops;
	}
}

void E64::kernel_t::timer_0_event()
{
	terminal->timer_callback();
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
