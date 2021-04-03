#include "hud.hpp"
#include "common.hpp"

E64::hud_t::hud_t()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_math(L);
	luaopen_string(L);
	
	blitter = new blitter_ic();
	cia = new cia_ic();
	timer = new timer_ic();
	
	stats_view = new tty_t(0b10001010, 0b00000000, 0x25, 0, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	terminal = new tty_t(0b10001010, 0b00000000, 0x46, 1, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	cpu_view = new tty_t(0b10001010, 0b00000000, 0x25, 2, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	disassembly_view = new tty_t(0b10001010, 0b00000000, 0x45, 3, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	stack_view = new tty_t(0b10001010, 0b00000000, 0x35, 4, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	bar_single_height = new tty_t(0b00001111, 0b00000000, 0x06, 5, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	bar_double_height = new tty_t(0b10001010, 0b00000000, 0x16, 6, blitter, GREEN_05, (GREEN_02 & 0x0fff) | 0xa000);
	
	stats_visible = false;
	overhead_visible = false;
	refresh = false;
}

E64::hud_t::~hud_t()
{
	delete bar_double_height;
	delete bar_single_height;
	delete stack_view;
	delete disassembly_view;
	delete cpu_view;
	delete terminal;
	delete stats_view;
	
	delete timer;
	delete cia;
	delete blitter;
	
	lua_close(L);
}

bool E64::hud_t::refreshed()
{
	bool return_value = refresh;
	if (refresh) refresh = false;
	return return_value;
}

void E64::hud_t::reset()
{
	blitter->reset();
	blitter->set_clear_color(0x0000);
	blitter->set_border_color(0x0000);
	blitter->set_border_size(0);
	
	cia->reset();
	cia->set_keyboard_repeat_delay(50);
	cia->set_keyboard_repeat_speed(5);
	cia->generate_key_events();
	
	timer->set(0, 3600);	// check keyboard state etc...
	timer->set(1, 3600);	// connected to execute
	
	terminal->clear();
	terminal->printf("E64 Virtual Computer System (C)%u elmerucr\n", E64_SQ_YEAR);
	terminal->puts(LUA_COPYRIGHT);
	terminal->prompt();
	terminal->activate_cursor();
	
	stack_view->clear();
	bar_single_height->clear();
	for (int i = 1536; i<2048; i++)
		blitter->blit[bar_single_height->blit_no].pixel_data[i] = GREEN_05;
	
	bar_double_height->clear();
}

void E64::hud_t::process_keypress()
{
	uint8_t key_value = cia->read_byte(0x04);
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
		case ASCII_F1:
			// NEEDS WORK
			if (machine.paused) {
				machine.cpu->run(0);
			}
			break;
		case ASCII_LF:
			{
				char *buffer = terminal->enter_command();
				terminal->putchar('\n');
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

void E64::hud_t::execute()
{
	while (cia->read_byte(0x00)) {
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
		uint16_t old_color = terminal->current_foreground_color;
		if (machine.cpu->breakpoints[pc] == true) disassembly_view->current_foreground_color = AMBER_07;
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
		disassembly_view->current_foreground_color = old_color;
	}
	refresh = true;
}

void E64::hud_t::run(uint16_t cycles)
{
	timer->run(cycles);
	if (timer->irq_line == false) {
		for (int i=0; i<8; i++) {
			if (timer->read_byte(0x00) & (0b1 << i)) {
				switch (i) {
					case 0:
						timer_0_event();
						timer->write_byte(0x00, 0b00000001);
						break;
					case 1:
						timer_1_event();
						timer->write_byte(0x00, 0b00000010);
					default:
						break;
				}
			}
		}
	}
	
	cia->run(cycles);
}

void E64::hud_t::timer_0_event()
{
	terminal->timer_callback();
}

void E64::hud_t::timer_1_event()
{
	execute();
}

void E64::hud_t::timer_2_event()
{
	//
}

void E64::hud_t::timer_3_event()
{
	//
}

void E64::hud_t::timer_4_event()
{
	//
}

void E64::hud_t::timer_5_event()
{
	//
}

void E64::hud_t::timer_6_event()
{
	//
}

void E64::hud_t::timer_7_event()
{
	//
}

void E64::hud_t::redraw()
{
	if (stats_visible && (!overhead_visible))
		blitter->draw_blit(hud.stats_view->blit_no, 128, 244);
	if (hud.overhead_visible) {
		blitter->draw_blit(hud.stats_view->blit_no, 0, 244);
		blitter->draw_blit(hud.terminal->blit_no, 0, 12);
		blitter->draw_blit(hud.cpu_view->blit_no, 0, 148);
		blitter->draw_blit(hud.stack_view->blit_no, 0, 180);
		blitter->draw_blit(hud.disassembly_view->blit_no, 256, 148);
		blitter->draw_blit(hud.bar_single_height->blit_no, 0, 140);
		blitter->draw_blit(hud.bar_double_height->blit_no, 0, -4);
		blitter->draw_blit(hud.bar_double_height->blit_no, 0, 276);
	}
}
