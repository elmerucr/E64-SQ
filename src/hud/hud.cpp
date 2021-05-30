#include "hud.hpp"
#include "common.hpp"
#include "sdl2.hpp"

/*
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 * from https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
 *
 * This function is slightly adopted to check for true values. It returns false
 * when there's wrong input.
 */
bool E64::hud_t::hex_string_to_int(const char *temp_string, uint32_t *return_value)
{
	uint32_t val = 0;
	while (*temp_string) {
		/* Get current character then increment */
		uint8_t byte = *temp_string++;
		/* Transform hex character to the 4bit equivalent number */
		if (byte >= '0' && byte <= '9') {
			byte = byte - '0';
		} else if (byte >= 'a' && byte <='f') {
			byte = byte - 'a' + 10;
		} else if (byte >= 'A' && byte <='F') {
			byte = byte - 'A' + 10;
		} else {
			/* Problem, return false and do not write the return value */
			return false;
		}
		/* Shift 4 to make space for new digit, and add the 4 bits of the new digit */
		val = (val << 4) | (byte & 0xf);
	}
	*return_value = val;
	return true;
}

E64::hud_t::hud_t()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_math(L);
	luaopen_string(L);
	
	exceptions = new exceptions_ic();
	blitter = new blitter_ic();
	cia = new cia_ic();
	timer = new timer_ic(exceptions);
	
	stats_view = new tty_t(0b10001010,
			       0b00000000,
			       0x25,
			       0,
			       blitter,
			       GREEN_05,
			       (GREEN_02 & 0x0fff) | 0xa000);
	
	tiepding = &blitter->blit[1];
	tiepding->clear();
	terminal = new tty_t(0b10001010,
			     0b00000000,
			     0x46,
			     1,
			     blitter,
			     GREEN_05,
			     (GREEN_02 & 0x0fff) | 0xa000);
	tiepding->terminal_init(0b10001010, 0b0, 0x36, GREEN_05, GREEN_02);
	
	
	cpu_view = new tty_t(0b10001010,
			     0b00000000,
			     0x15,
			     2,
			     blitter,
			     GREEN_05,
			     (GREEN_02 & 0x0fff) | 0xa000);
	
	disassembly_view = new tty_t(0b10001010,
				     0b00000000,
				     0x45,
				     3,
				     blitter,
				     GREEN_05,
				     (GREEN_02 & 0x0fff) | 0xa000);
	
	stack_view = new tty_t(0b10001010,
			       0b00000000,
			       0x34,
			       4,
			       blitter,
			       GREEN_05,
			       (GREEN_02 & 0x0fff) | 0xa000);
	
	bar_single_height = new tty_t(0b00001111,
				      0b00000000,
				      0x06,
				      5,
				      blitter,
				      GREEN_05,
				      (GREEN_02 & 0x0fff) | 0xa000);
	
	bar_double_height = new tty_t(0b10001010,
				      0b00000000,
				      0x16,
				      6,
				      blitter,
				      GREEN_05,
				      (GREEN_02 & 0x0fff) | 0xa000);
	
	bar_single_height_small_1 = new tty_t(0b10001010,
				      0b00000000,
				      0x05,
				      7,
				      blitter,
				      GREEN_05,
				      (GREEN_02 & 0x0fff) | 0xa000);
	
	bar_single_height_small_2 = new tty_t(0b10001010,
				      0b00000000,
				      0x05,
				      8,
				      blitter,
				      GREEN_05,
				      (GREEN_02 & 0x0fff) | 0xa000);
	
	other_info = new tty_t(0b10001010,
			       0b00000000,
			       0x34,
			       9,
			       blitter,
			       GREEN_05,
			       (GREEN_02 & 0x0fff) | 0xa000);
	
	stats_visible = false;
	irq_line = true;
}

E64::hud_t::~hud_t()
{
	delete other_info;
	delete bar_single_height_small_1;
	delete bar_single_height_small_2;
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
	delete exceptions;
	
	lua_close(L);
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
	
	timer->reset();
	timer->set(0, 3600);	// check keyboard, generate key events...
	
	terminal->clear();
	terminal->printf("E64 Virtual Computer System (C)%u elmerucr", E64_YEAR);
	//terminal->puts(LUA_COPYRIGHT);
	terminal->prompt();
	terminal->activate_cursor();
	
	bar_single_height->clear();
	for (int i=0; i<4096; i++)
		blitter->blit[bar_single_height->blit_no].pixel_data[i] = 0x0000;
	for (int i = 1536; i<2048; i++)
		blitter->blit[bar_single_height->blit_no].pixel_data[i] = GREEN_05;
	
	bar_double_height->clear();
	bar_single_height_small_1->clear();
	bar_single_height_small_1->puts("                           | |");
	bar_single_height_small_2->clear();
	
	other_info->clear();
}

void E64::hud_t::process_keypress()
{
	while (cia->read_byte(0x00)) {
		terminal->deactivate_cursor();
	
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
				terminal->deactivate_cursor();
				if (machine.paused) {
					machine.run(0);
				}
				terminal->activate_cursor();
				break;
			case ASCII_F2:
				terminal->deactivate_cursor();
				if (machine.paused) {
					machine.run(0);
					machine.run(0);
				}
				terminal->activate_cursor();
				break;
			case ASCII_F3:
				terminal->deactivate_cursor();
				if (machine.paused) {
					machine.run(0);
					machine.run(0);
					machine.run(0);
					machine.run(0);
				}
				terminal->activate_cursor();
				break;
			case ASCII_LF:
			{
				char *buffer = terminal->enter_command();
				process_command(buffer);
			}
				break;
			default:
				terminal->putchar(key_value);
				break;
		}
		terminal->activate_cursor();
	}
}

void E64::hud_t::update_stats_view()
{
	stats_view->clear();
	stats_view->puts(stats.summary());
}

void E64::hud_t::update()
{
	cpu_view->clear();
	cpu_view->printf("  pc  ac xr yr sp nv-bdizc I N\n");
	cpu_view->printf(" %04x %02x %02x %02x %02x %c%c%c%c%c%c%c%c %c %c",
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
			 machine.exceptions->irq_output_pin ? '1' : '0',
			 machine.exceptions->nmi_output_pin ? '1' : '0');
	
	cpu_view->text_screen->foreground_color = GREEN_03;
	cpu_view->putchar(machine.cpu->get_old_nmi_line() ? '1' : '0');
	cpu_view->text_screen->foreground_color = GREEN_05;
	
	disassembly_view->clear();
	char text_buffer[256];
	uint16_t pc = machine.cpu->get_pc();
	for (int i=0; i<16; i++) {
		uint16_t old_color = terminal->text_screen->foreground_color;
		if (machine.cpu->breakpoint[pc] == true) disassembly_view->text_screen->foreground_color = AMBER_07;
		if (disassembly_view->get_current_column() != 0)
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
		disassembly_view->text_screen->foreground_color = old_color;
	}
	
	stack_view->clear();
	uint8_t temp_sp = machine.cpu->get_sp();
	
	stack_view->text_screen->foreground_color = GREEN_03;
	stack_view->printf("  %04x: %02x %04x\n",
			   0x0100 | temp_sp,
			   machine.mmu->read_memory_8(0x0100 | temp_sp),
			   machine.mmu->read_memory_8(0x0100 | temp_sp) |
			   machine.mmu->read_memory_8(0x0100 | ((temp_sp+1) & 0xff)) << 8);
	stack_view->text_screen->foreground_color = GREEN_05;
	temp_sp++;
	
	for (int i=0; i<6; i++) {
		stack_view->printf("  %04x: %02x %04x\n",
				   0x0100 | temp_sp,
				   machine.mmu->read_memory_8(0x0100 | temp_sp),
				   machine.mmu->read_memory_8(0x0100 | temp_sp) |
				   machine.mmu->read_memory_8(0x0100 | ((temp_sp+1) & 0xff)) << 8);
		temp_sp++;
	}
	stack_view->printf("  %04x: %02x %04x",
			   0x0100 | temp_sp,
			   machine.mmu->read_memory_8(0x0100 | temp_sp),
			   machine.mmu->read_memory_8(0x0100 | temp_sp) |
			   machine.mmu->read_memory_8(0x0100 | ((temp_sp+1) & 0xff)) << 8);
	
	other_info->clear();
	other_info->printf("           | |\n"
			   "    vicv-%c-+ |\n"
			   "   timer-%c-+ |\n"
			   "     hud-%c-+ |\n"
			   "             |\n"
			   "     xxx-1---+\n",
			   machine.exceptions->irq_input_pins[vicv.irq_number] ? '1' : '0',
			   machine.exceptions->irq_input_pins[machine.timer->irq_number] ? '1' : '0',
			   irq_line ? '1' : '0');
}

void E64::hud_t::run(uint16_t cycles)
{
	timer->run(cycles);
	if (exceptions->irq_output_pin == false) {
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
	terminal->process_cursor_state();
}

void E64::hud_t::timer_1_event()
{

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
	if (stats_visible && paused)
		blitter->draw_blit(hud.stats_view->blit_no, 128, 244);
	if (!paused) {
		blitter->draw_blit(hud.stats_view->blit_no, 0, 244);
		blitter->draw_blit(hud.bar_single_height_small_2->blit_no, 0, 236);
		blitter->draw_blit(hud.terminal->blit_no, 0, 12);
		blitter->draw_blit(hud.cpu_view->blit_no, 0, 148);
		blitter->draw_blit(hud.bar_single_height_small_1->blit_no, 0, 164);
		blitter->draw_blit(hud.stack_view->blit_no, 0, 172);
		blitter->draw_blit(hud.disassembly_view->blit_no, 256, 148);
		blitter->draw_blit(hud.bar_single_height->blit_no, 0, 140);
		blitter->draw_blit(hud.bar_double_height->blit_no, 0, -4);
		blitter->draw_blit(hud.bar_double_height->blit_no, 0, 276);
		blitter->draw_blit(hud.other_info->blit_no, 128, 172);
	}
}

void E64::hud_t::flip_modes()
{
	machine.paused = !machine.paused;
	if (machine.paused) sdl2_stop_audio();
	hud.paused = !machine.paused;
}

void E64::hud_t::process_command(char *buffer)
{
	bool have_prompt = true;
	
	char *token0, *token1;
	token0 = strtok(buffer, " ");
	
	if (token0 == NULL) {
		have_prompt = false;
		terminal->putchar('\n');
	} else if (token0[0] == ':') {
		have_prompt = false;
		enter_monitor_line(buffer);
	} else if (token0[0] == ';') {
		have_prompt = false;
		enter_monitor_blit_line(buffer);
	} else if (strcmp(token0, "b") == 0) {
		token1 = strtok(NULL, " ");
		terminal->putchar('\n');
		if (token1 == NULL) {
			uint16_t count = 0;
			for (int i=0; i< RAM_SIZE; i++) {
				if (machine.cpu->breakpoint[i]) {
					terminal->printf("%04x ", i);
					count++;
					if ((count % 4) == 0)
						terminal->putchar('\n');
				}
			}
			if (count == 0) {
				terminal->puts("no breakpoints");
			}
		} else {
			uint32_t temp_32bit;
			if (hex_string_to_int(token1, &temp_32bit)) {
				temp_32bit &= (RAM_SIZE - 1);
				machine.cpu->breakpoint[temp_32bit] =
					!machine.cpu->breakpoint[temp_32bit];
				terminal->printf("breakpoint %s at $%04x",
						machine.cpu->breakpoint[temp_32bit] ? "set" : "cleared",
						temp_32bit);
			} else {
				terminal->puts("error: invalid address\n");
			}
		}
	} else if (strcmp(token0, "bc") == 0 ) {
		terminal->puts("\nclearing all breakpoints");
		machine.cpu->clear_breakpoints();
	} else if (strcmp(token0, "bm") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();

		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t blit_memory_location = 0x000000;

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				blit_memory_dump(blit_memory_location, 1);
				blit_memory_location = (blit_memory_location + 8) & 0xfffff8;
			}
		} else {
			if (!hex_string_to_int(token1, &blit_memory_location)) {
				terminal->putchar('\n');
				terminal->puts("error: invalid address\n");
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					blit_memory_dump(blit_memory_location & 0x00fffff8, 1);
					blit_memory_location = (blit_memory_location + 8) & 0xfffff8;
				}
			}
		}
	} else if (strcmp(token0, "c") == 0 ) {
		flip_modes();
	} else if (strcmp(token0, "clear") == 0 ) {
		have_prompt = false;
		terminal->clear();
	} else if (strcmp(token0, "exit") == 0) {
		have_prompt = false;
		E64::sdl2_wait_until_enter_released();
		app_running = false;
	} else if (strcmp(token0, "m") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");
		
		uint8_t lines_remaining = terminal->lines_remaining();
		
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc = machine.cpu->get_pc();
	
		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				memory_dump(temp_pc, 1);
				temp_pc = (temp_pc + 8) & 0xffff;
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->putchar('\n');
				terminal->puts("error: invalid address\n");
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					memory_dump(temp_pc & (RAM_SIZE - 1), 1);
					temp_pc = (temp_pc + 8) & 0xffff;
				}
			}
		}
	} else if (strcmp(token0, "reset") == 0) {
		E64::sdl2_wait_until_enter_released();
		machine.reset();
	} else if (strcmp(token0, "timers") == 0) {
		for (int i=0; i<8; i++) {
			char text_buffer[64];
			machine.timer->status(text_buffer, i);
			terminal->puts(text_buffer);
		}
	} else if (strcmp(token0, "ver") == 0) {
		terminal->printf("\nE64 (C)%i - version %i.%i (%i)", E64_YEAR, E64_MAJOR_VERSION, E64_MINOR_VERSION, E64_BUILD);
	} else {
		terminal->putchar('\n');
		terminal->printf("error: unknown command '%s'", token0);
	}
	if (have_prompt) terminal->prompt();
}

void E64::hud_t::memory_dump(uint16_t address, int rows)
{
    address = address & 0xffff;  // only even addresses allowed
    
	for (int i=0; i<rows; i++ ) {
		uint16_t temp_address = address;
		terminal->printf("\r:%04x ", temp_address);
		for (int i=0; i<8; i++) {
			terminal->printf("%02x ", machine.mmu->read_memory_8(temp_address));
			temp_address++;
			temp_address &= RAM_SIZE - 1;
		}
	
		terminal->text_screen->foreground_color = GREEN_06;
		terminal->text_screen->background_color = (GREEN_02 & 0x0fff) | 0xc000;
		
		temp_address = address;
		for (int i=0; i<8; i++) {
			uint8_t temp_byte = machine.mmu->read_memory_8(temp_address);
			terminal->putsymbol(temp_byte);
			temp_address++;
		}
		address += 8;
		address &= RAM_SIZE - 1;
	
		terminal->text_screen->foreground_color = GREEN_05;
		terminal->text_screen->background_color = (GREEN_02 & 0x0fff) | 0xa000;
       
		for (int i=0; i<32; i++) terminal->cursor_left();
	}
}

void E64::hud_t::blit_memory_dump(uint32_t address, int rows)
{
    address = address & 0xfffff8;
    
	for (int i=0; i<rows; i++ ) {
		uint32_t temp_address = address;
		terminal->printf("\r;%06x ", temp_address);
		for (int i=0; i<8; i++) {
			terminal->printf("%02x ", machine.blitter->read_memory_8(temp_address));
			temp_address++;
		}
	
		terminal->text_screen->foreground_color = GREEN_06;
		terminal->text_screen->background_color = (GREEN_02 & 0x0fff) | 0xc000;
		
		temp_address = address;
		for (int i=0; i<8; i++) {
			uint8_t temp_byte = machine.blitter->read_memory_8(temp_address);
			terminal->putsymbol(temp_byte);
			temp_address++;
		}
		address += 8;
		address &= 0x00fffff8;
	
		terminal->text_screen->foreground_color = GREEN_05;
		terminal->text_screen->background_color = (GREEN_02 & 0x0fff) | 0xa000;
       
		for (int i=0; i<32; i++) terminal->cursor_left();
	}
}

void E64::hud_t::enter_monitor_line(char *buffer)
{
	uint32_t address;
	uint32_t arg0, arg1, arg2, arg3;
	uint32_t arg4, arg5, arg6, arg7;
    
	buffer[5]  = '\0';
	buffer[8]  = '\0';
	buffer[11] = '\0';
	buffer[14] = '\0';
	buffer[17] = '\0';
	buffer[20] = '\0';
	buffer[23] = '\0';
	buffer[26] = '\0';
	buffer[29] = '\0';
    
	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->puts("????\n");
	} else if (!hex_string_to_int(&buffer[6], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<6; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[9], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<9; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[12], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<12; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[15], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<15; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[18], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<18; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[21], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<21; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[24], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<24; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[27], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<27; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else {
		uint16_t original_address = address;
	
		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;
		arg4 &= 0xff;
		arg5 &= 0xff;
		arg6 &= 0xff;
		arg7 &= 0xff;
	
		machine.mmu->write_memory_8(address, (uint8_t)arg0); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg1); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg2); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg3); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg4); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg5); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg6); address +=1; address &= 0xffff;
		machine.mmu->write_memory_8(address, (uint8_t)arg7); address +=1; address &= 0xffff;

		terminal->putchar('\r');
	
		memory_dump(original_address, 1);
	
		original_address += 8;
		original_address &= 0xffff;
		terminal->printf("\n:%04x ", original_address);
	}
}

void E64::hud_t::enter_monitor_blit_line(char *buffer)
{
	uint32_t address;
	uint32_t arg0, arg1, arg2, arg3;
	uint32_t arg4, arg5, arg6, arg7;
    
	buffer[7]  = '\0';
	buffer[10]  = '\0';
	buffer[13] = '\0';
	buffer[16] = '\0';
	buffer[19] = '\0';
	buffer[22] = '\0';
	buffer[25] = '\0';
	buffer[28] = '\0';
	buffer[31] = '\0';
    
	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->puts("??????\n");
	} else if (!hex_string_to_int(&buffer[8], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<8; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[11], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<11; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[14], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<14; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[17], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<17; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[20], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<20; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[23], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<23; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[26], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<26; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else if (!hex_string_to_int(&buffer[29], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<29; i++) terminal->cursor_right();
		terminal->puts("??\n");
	} else {
		uint32_t original_address = address;
	
		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;
		arg4 &= 0xff;
		arg5 &= 0xff;
		arg6 &= 0xff;
		arg7 &= 0xff;
	
		machine.blitter->write_memory_8(address, (uint8_t)arg0); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg1); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg2); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg3); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg4); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg5); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg6); address +=1; address &= 0xffffff;
		machine.blitter->write_memory_8(address, (uint8_t)arg7); address +=1; address &= 0xffffff;

		terminal->putchar('\r');
	
		blit_memory_dump(original_address, 1);
	
		original_address += 8;
		original_address &= 0xffffff;
		terminal->printf("\n;%06x ", original_address);
	}
}
