/*
 * mmu.cpp
 * E64-II
 *
 * Copyright © 2019-2020 elmerucr. All rights reserved.
 */

#include "mmu.hpp"
#include "common.hpp"

E64::mmu_ic::mmu_ic()
{
	// allocate main ram and fill with a pattern
	ram = new uint8_t[RAM_SIZE * sizeof(uint8_t)];
	ram_as_words = (uint16_t *)ram;
	reset();
}

E64::mmu_ic::~mmu_ic()
{
	delete ram;
	ram = nullptr;
}

void E64::mmu_ic::reset()
{
	// fill alternating blocks with 0x00 and 0x80
	for (int i=0; i<RAM_SIZE; i++)
		ram[i] = (i & 64) ? 0x10 : 0x00;
}

unsigned int E64::mmu_ic::read_memory_8(unsigned int address)
{
	uint32_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		return machine.vicv->read_byte(address & 0xff);
	} else if (page == IO_SND_PAGE) {
		return machine.sids->read_byte(address & 0xff);
	} else if (page == IO_BLITTER_PAGE) {
		return machine.blitter->read_byte(address & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		return machine.timer->read_byte(address & 0xff);
	} else if (page == IO_CIA_PAGE) {
		return machine.cia->read_byte(address & 0xff);
	} else if ((address & IO_FONT) == IO_FONT) {
		return cbm_cp437_font[address & 0x7ff];
	} else {
		return ram[address & 0xffffff];
	}
}

unsigned int E64::mmu_ic::read_memory_16(unsigned int address)
{
	unsigned int result;
	uint32_t temp_address = address;
	result = read_memory_8(temp_address);
	temp_address++;
	result = read_memory_8(temp_address) | (result << 8);
	return result;
}

unsigned int E64::mmu_ic::read_disassembler_8(unsigned int address)
{
	return read_memory_8(address);
}

unsigned int E64::mmu_ic::read_disassembler_16(unsigned int address)
{
	return read_memory_16(address);
}

void E64::mmu_ic::write_memory_8(unsigned int address, unsigned int value)
{
	uint32_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		machine.vicv->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_SND_PAGE) {
		machine.sids->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_BLITTER_PAGE) {
		machine.blitter->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		machine.timer->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_CIA_PAGE) {
		machine.cia->write_byte(address & 0xff, value & 0xff);
	} else {
		ram[address & 0xffffff] = value & 0xff;
	}
}

void E64::mmu_ic::write_memory_16(unsigned int address, unsigned int value)
{
	uint32_t temp_address = address;
	write_memory_8(temp_address, value >> 8);
	temp_address++;
	write_memory_8(temp_address, value & 0xff);
}
