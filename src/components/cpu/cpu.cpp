#include "cpu.hpp"
#include <cstdio>
#include <cstring>

extern "C" {
#include "fake6502.h"
}

#include "mnemonics.h"

cpu_ic::cpu_ic()
{
//	*irq_line = true;
//	*nmi_line = true;
	old_nmi_line = true;

	breakpoint = nullptr;
	breakpoint = new bool[65536];
	clear_breakpoints();
}

cpu_ic::~cpu_ic()
{
	delete [] breakpoint;
}

void cpu_ic::reset()
{
	reset6502();
	cycle_saldo = 0;
}

void cpu_ic::clear_breakpoints()
{
	if (breakpoint) {
		for (int i=0; i<65536; i++) breakpoint[i] = false;
	}
}

void cpu_ic::toggle_breakpoint(uint16_t address)
{
	breakpoint[address] = !breakpoint[address];
}

bool cpu_ic::run(int32_t desired_cycles, int32_t *consumed_cycles)
{
	cycle_saldo += desired_cycles;
	
	bool breakpoint_reached = false;
	
	*consumed_cycles = 0;

	/*
	 * This loop runs always at least one instruction. If an irq or nmi is
	 * triggered, that operation is run instead.
	 */
	do {
		uint32_t old_clockticks6502 = clockticks6502;
		if ((*nmi_line == false) && (old_nmi_line = true)) {
			nmi6502();
			*consumed_cycles += 7;
		} else if (!(*irq_line) && !(status & FLAG_INTERRUPT)) {
			irq6502();
			*consumed_cycles += 7;
		} else {
			step6502();
		}
		*consumed_cycles += (clockticks6502 - old_clockticks6502);
		breakpoint_reached = breakpoint[pc];
	} while ((*consumed_cycles < cycle_saldo) && (!breakpoint_reached) && (desired_cycles > 0));
	

	old_nmi_line = nmi_line;
	
	cycle_saldo -= *consumed_cycles;
	
	return breakpoint_reached;
}

uint32_t cpu_ic::clock_ticks()
{
	return clockticks6502;
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

void cpu_ic::assign_irq_pin(bool *pin)
{
	irq_line = pin;
}

void cpu_ic::assign_nmi_pin(bool *pin)
{
	nmi_line = pin;
}




//
//
//bool cpu_ic::run(int32_t desired_cycles, int32_t *consumed_cycles)
//{
//	cycle_saldo += desired_cycles;
//
//	bool done = false;
//	bool breakpoint_reached = false;
//
//	*consumed_cycles = 0;
//
//	if ((*nmi_line == false) && (old_nmi_line = true)) {
//		nmi6502();
//		*consumed_cycles += 7;
//		if (*consumed_cycles >= cycle_saldo) done = true;
//		if (breakpoint[pc]) {
//			breakpoint_reached = true;
//			done = true;
//		}
//	} else if (!(*irq_line) && !(status & FLAG_INTERRUPT)) {
//		irq6502();
//		*consumed_cycles += 7;
//		if (*consumed_cycles >= cycle_saldo) done = true;
//		if (breakpoint[pc]) {
//			breakpoint_reached = true;
//			done = true;
//		}
//	}
//
//	/*
//	 * This loop runs at least one instruction, unless a breakpoint was
//	 * already detected at the start of an interrupt routine
//	 */
//	if (!done) {
//		do {
//			uint32_t old_clockticks6502 = clockticks6502;
//			step6502();
//			*consumed_cycles += (clockticks6502 - old_clockticks6502);
//			breakpoint_reached = breakpoint[pc];
//		} while ((*consumed_cycles < cycle_saldo) && (!breakpoint_reached) && (desired_cycles > 0));
//	}
//
//	old_nmi_line = nmi_line;
//
//	cycle_saldo -= *consumed_cycles;
//
//	return breakpoint_reached;
//}
