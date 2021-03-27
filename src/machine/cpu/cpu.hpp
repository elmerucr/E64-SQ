#ifndef CPU_HPP
#define CPU_HPP

#include <cstdlib>

#define FLAG_CARRY     0x01
#define FLAG_ZERO      0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL   0x08
#define FLAG_BREAK     0x10
#define FLAG_CONSTANT  0x20
#define FLAG_OVERFLOW  0x40
#define FLAG_SIGN      0x80

class cpu_ic {
private:
	bool irq_line;
	bool old_irq_line;
	bool nmi_line;
	bool old_nmi_line;

	bool *breakpoints;
public:
	cpu_ic();
	~cpu_ic();

	void reset();

	/*
	 * Basic run function. When run with 0 cycles, it runs 1 instruction
	 * minimal. The return value contains the realized number of cycles.
	 */
	uint32_t run(uint32_t cycles);

	void pull_irq();
	void release_irq();
	void pull_nmi();
	void release_nmi();

	uint16_t get_pc();
	uint8_t  get_sp();
	uint8_t  get_a();
	uint8_t  get_x();
	uint8_t  get_y();
	uint8_t  get_status();

	void set_pc(uint16_t _pc);
	void set_sp(uint8_t _sp);
	void set_a(uint8_t _a);
	void set_x(uint8_t _x);
	void set_y(uint8_t _y);
	void set_status(uint8_t _status);

	int disassemble(char *text);
	int disassemble(uint16_t _pc, char *text);

	void dump_stack();
	uint32_t clock_ticks();
};

#endif
