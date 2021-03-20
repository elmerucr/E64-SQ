/*
 * mmu.cpp
 * E64
 *
 * Copyright © 2019-2021 elmerucr. All rights reserved.
 */

#include "mmu.hpp"
#include "common.hpp"

E64::mmu_ic::mmu_ic()
{
	ram = new uint8_t[RAM_SIZE * sizeof(uint8_t)];
	//ram_as_words = (uint16_t *)ram;
	reset();
}

E64::mmu_ic::~mmu_ic()
{
	delete ram;
	ram = nullptr;
}

void E64::mmu_ic::reset()
{
	// fill alternating blocks with 0x00 and 0x10
	for (int i=0; i<RAM_SIZE; i++)
		ram[i] = (i & 64) ? 0x10 : 0x00;
}

uint8_t E64::mmu_ic::read_memory_8(uint16_t address)
{
	uint16_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		return machine.vicv->read_byte(address & 0xff);
	} else if (page == IO_SND_PAGE) {
		return machine.sids->read_byte(address & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		return machine.timer->read_byte(address & 0xff);
	} else if (page == IO_CIA_PAGE) {
		return machine.cia->read_byte(address & 0xff);
	} else {
		return ram[address & 0xffff];
	}
}

void E64::mmu_ic::write_memory_8(uint16_t address, uint8_t value)
{
	uint16_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		machine.vicv->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_SND_PAGE) {
		machine.sids->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		machine.timer->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_CIA_PAGE) {
		machine.cia->write_byte(address & 0xff, value & 0xff);
	} else {
		ram[address & 0xffff] = value & 0xff;
	}
}
