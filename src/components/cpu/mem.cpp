/*
 * Glue code to get c library working with c++ class members
 */

#include <cstdint>
#include "common.hpp"

extern "C" uint8_t read6502(uint16_t address)
{
	return machine.mmu->read_memory_8(address);
}

extern "C" void write6502(uint16_t address, uint8_t value)
{
	machine.mmu->write_memory_8(address, value);
}
