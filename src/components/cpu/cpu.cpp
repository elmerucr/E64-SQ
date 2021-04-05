#include "cpu.hpp"
#include <cstdio>
#include <cstring>

extern "C" {
#include "fake6502.h"
}

#include "mnemonics.h"

cpu_ic::cpu_ic()
{
	irq_line = true;
	nmi_line = true;
	old_nmi_line = true;

	breakpoints = nullptr;
	breakpoints = new bool[65536];
	clear_breakpoints();
	//breakpoint_reached = false;
}

cpu_ic::~cpu_ic()
{
	delete [] breakpoints;
}

void cpu_ic::reset()
{
	reset6502();
	cycle_saldo = 0;
}

void cpu_ic::clear_breakpoints()
{
	if (breakpoints) {
		for (int i=0; i<65536; i++) breakpoints[i] = false;
	}
}

void cpu_ic::toggle_breakpoint(uint16_t address)
{
	breakpoints[address] = !breakpoints[address];
}

uint32_t cpu_ic::run(uint32_t cycles)
{
	cycle_saldo += cycles;
	
	bool done = false;
	bool breakpoint_reached = false;

	int32_t cycles_done = 0;

	if ((nmi_line == false) && (old_nmi_line = true)) {
		nmi6502();
		cycles_done += 7;
		if (cycles_done >= cycle_saldo) done = true;
		if (breakpoints[pc]) {
			breakpoint_reached = true;
			done = true;
		}
	} else if (!irq_line && !(status & FLAG_INTERRUPT)) {
		irq6502();
		cycles_done += 7;
		if (cycles_done >= cycle_saldo) done = true;
		if (breakpoints[pc]) {
			breakpoint_reached = true;
			done = true;
		}
	}

	/*
	 * This loop always runs one instruction, unless breakpoint was
	 * already detected.
	 */
	if (!done) {
		do {
			uint32_t old_clockticks6502 = clockticks6502;
			step6502();
			cycles_done += (clockticks6502 - old_clockticks6502);
			if (breakpoints[pc]) {
				breakpoint_reached = true;
			}
		} while ((cycles_done < cycle_saldo) && !breakpoint_reached);
	}

	old_nmi_line = nmi_line;
	
	cycle_saldo -= cycles_done;

	return cycles_done;
}

uint32_t cpu_ic::clock_ticks()
{
	return clockticks6502;
}

void cpu_ic::pull_irq()
{
	irq_line = false;
}

void cpu_ic::release_irq()
{
	irq_line = true;
}

void cpu_ic::dump_stack()
{
	printf("Stack dump\n");
	for (int i=0; i < 10; i++) {
		printf("%04x %02x\n", 0x100+sp+i, read6502(0x100+sp+i));
	}
}

uint16_t cpu_ic::get_pc()     { return     pc; }
uint8_t  cpu_ic::get_sp()     { return     sp; }
uint8_t  cpu_ic::get_a()      { return      a; }
uint8_t  cpu_ic::get_x()      { return      x; }
uint8_t  cpu_ic::get_y()      { return      y; }
uint8_t  cpu_ic::get_status() { return status; }

void cpu_ic::set_pc(uint16_t _pc)        { pc = _pc; }
void cpu_ic::set_sp(uint8_t _sp)         { sp = _sp; }
void cpu_ic::set_a(uint8_t _a)           { a = _a; }
void cpu_ic::set_x(uint8_t _x)           { x = _x; }
void cpu_ic::set_y(uint8_t _y)           { y = _y; }
void cpu_ic::set_status(uint8_t _status) { status = _status; }

int cpu_ic::disassemble(uint16_t _pc, char *buffer)
{
	//char buffer[256];
	uint8_t opcode = read6502(_pc);
	char const *mnemonic = mnemonics[opcode];

	// Test for branches, relative address. These are BRA ($80) and
	// $10,$30,$50,$70,$90,$B0,$D0,$F0.
	bool is_branch = (opcode == 0x80) || ((opcode & 0x1f) == 0x10);

	// Ditto bbr and bbs, the "zero-page, relative" ops.
	// $0F,$1F,$2F,$3F,$4F,$5F,$6F,$7F,$8F,$9F,$AF,$BF,$CF,$DF,$EF,$FF
	bool is_zp_rel = (opcode & 0x0f) == 0x0f;

	int length = 1;

	strncpy(buffer, mnemonic, 256);

	if (is_zp_rel) {
		snprintf(buffer, 256, mnemonic, read6502(_pc + 1), _pc + 3 + (int8_t)read6502(_pc + 2));
		length = 3;
	} else {
		if (strstr(buffer, "%02x")) {
			length = 2;
			if (is_branch) {
				snprintf(buffer, 256, mnemonic, _pc + 2 + (int8_t)read6502(_pc + 1));
			} else {
				snprintf(buffer, 256, mnemonic, read6502(_pc + 1));
			}
		}
		if (strstr(buffer, "%04x")) {
			length = 3;
			snprintf(buffer, 256, mnemonic, read6502(_pc + 1) | read6502(_pc + 2) << 8);
		}
	}
	return length;
}

int cpu_ic::disassemble(char *buffer)
{
	return disassemble(pc, buffer);
}
