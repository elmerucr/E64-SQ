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
	if (breakpoints) {
		for (int i=0; i<65536; i++) breakpoints[i] = false;
	}
}

cpu_ic::~cpu_ic()
{
	delete [] breakpoints;
}

void cpu_ic::reset()
{
	reset6502();
}

uint32_t cpu_ic::run(uint32_t cycles)
{
	bool done = false;

	uint32_t cycles_done = 0;

	if ((nmi_line == false) && (old_nmi_line = true)) {
		nmi6502();
		cycles_done += 7;
		if (cycles_done >= cycles) done = true;
	} else if (!irq_line && !(status & FLAG_INTERRUPT)) {
		irq6502();
		cycles_done += 7;
		if (cycles_done >= cycles) done = true;
	}

	if (!done) {
		do {
			uint32_t old_clockticks6502 = clockticks6502;
			step6502();
			cycles_done += (clockticks6502 - old_clockticks6502);
		} while (cycles_done < cycles);
	}

	old_nmi_line = nmi_line;

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

int cpu_ic::disassemble(uint16_t _pc, char *text)
{
	//char buffer[256];
	uint8_t opcode = read6502(_pc);
	char const *mnemonic = mnemonics[opcode];

	// Test for branches, relative address. These are BRA ($80) and
	// $10,$30,$50,$70,$90,$B0,$D0,$F0.
	bool is_branch = (opcode == 0x80) || ((opcode & 0x1f) == 0x10);

	bool is_zp_rel = (opcode & 0x0f) == 0x0f;

	int length = 1;

	strncpy(text, mnemonic, 256);

	if (is_zp_rel) {
		snprintf(text, 256, mnemonic, read6502(_pc + 1), _pc + 3 + (int8_t)read6502(_pc + 2));
		length = 3;
	} else {
		if (strstr(text, "%02x")) {
			length = 2;
			if (is_branch) {
				snprintf(text, 256, mnemonic, _pc + 2 + (int8_t)read6502(_pc + 1));
			} else {
				snprintf(text, 256, mnemonic, read6502(_pc + 1));
			}
		}
		if (strstr(text, "%04x")) {
			length = 3;
			snprintf(text, 256, mnemonic, read6502(_pc + 1) | read6502(_pc + 2) << 8);
		}
	}
	//printf("%04x %s\n", _pc, buffer);
	return length;
}

// int disasm(uint16_t pc, uint8_t *RAM, char *line, unsigned int max_line, bool debugOn, uint8_t bank) {
// 	uint8_t opcode = real_read6502(pc, debugOn, bank);
// 	char const *mnemonic = mnemonics[opcode];

// 	//
// 	//		Test for branches, relative address. These are BRA ($80) and
// 	//		$10,$30,$50,$70,$90,$B0,$D0,$F0.
// 	//
// 	//
// 	int isBranch = (opcode == 0x80) || ((opcode & 0x1F) == 0x10);
// 	//
// 	//		Ditto bbr and bbs, the "zero-page, relative" ops.
// 	//		$0F,$1F,$2F,$3F,$4F,$5F,$6F,$7F,$8F,$9F,$AF,$BF,$CF,$DF,$EF,$FF
// 	//
// 	int isZprel  = (opcode & 0x0F) == 0x0F;

// 	int length   = 1;
// 	strncpy(line,mnemonic,max_line);

// 	if (isZprel) {
// 		snprintf(line, max_line, mnemonic, real_read6502(pc + 1, debugOn, bank), pc + 3 + (int8_t)real_read6502(pc + 2, debugOn, bank));
// 		length = 3;
// 	} else {
// 		if (strstr(line, "%02x")) {
// 			length = 2;
// 			if (isBranch) {
// 				snprintf(line, max_line, mnemonic, pc + 2 + (int8_t)real_read6502(pc + 1, debugOn, bank));
// 			} else {
// 				snprintf(line, max_line, mnemonic, real_read6502(pc + 1, debugOn, bank));
// 			}
// 		}
// 		if (strstr(line, "%04x")) {
// 			length = 3;
// 			snprintf(line, max_line, mnemonic, real_read6502(pc + 1, debugOn, bank) | real_read6502(pc + 2, debugOn, bank) << 8);
// 		}
// 	}
// 	return length;


int cpu_ic::disassemble(char *text)
{
	return disassemble(pc, text);
}
