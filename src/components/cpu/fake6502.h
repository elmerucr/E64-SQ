// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD

#ifndef _FAKE6502_H_
#define _FAKE6502_H_

#include <stdint.h>

extern uint16_t pc;
extern uint8_t sp, a, x, y, status;

extern void reset6502();
extern void step6502();
extern void exec6502(uint32_t tickcount);
extern void irq6502();
extern void nmi6502();
extern uint32_t clockticks6502;

// externally supplied functions
extern uint8_t read6502(uint16_t address);
extern void write6502(uint16_t address, uint8_t value);

#endif
