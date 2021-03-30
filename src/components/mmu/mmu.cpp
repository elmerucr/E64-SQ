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
	for (int i=0; i < RAM_SIZE; i++)
		ram[i] = (i & 64) ? 0x10 : 0x00;

	// try to find and update rom image
	find_and_update_rom_image();
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
	} else if ((page & IO_ROM_PAGE) == IO_ROM_PAGE) {
		return current_rom_image[address & 0x1fff];
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

void E64::mmu_ic::find_and_update_rom_image()
{
	FILE *f = fopen(host.settings.path_to_rom, "r");
	
	if (f) {
		printf("[mmu] found 'rom.bin' in %s, using this image\n",
		       host.settings.settings_path);
		fread(current_rom_image, 8192, 1, f);
		fclose(f);
	} else {
		printf("[mmu] no 'rom.bin' in %s, using built-in rom\n",
		       host.settings.settings_path);
		for(int i=0; i<8192; i++) current_rom_image[i] = rom[i];
	}
}
