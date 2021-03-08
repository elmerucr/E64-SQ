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
	
	heap_start = &ram[0x000000];
	heap_end = &ram[0xf00000];
	
	printf("[mmu allocation] size of memblock meta info: %lu\n",
	       sizeof(struct block));
	
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
	allocation_init();
}

unsigned int E64::mmu_ic::read_memory_8(unsigned int address)
{
	uint32_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		return machine.vicv->read_byte(address & 0xff);
	} else if (page == IO_SND_PAGE) {
		return machine.sids->read_byte(address & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		return machine.timer->read_byte(address & 0xff);
	} else if (page == IO_CIA_PAGE) {
		return machine.cia->read_byte(address & 0xff);
	} else {
		return ram[address & 0xffffff];
	}
}

//unsigned int E64::mmu_ic::read_memory_16(unsigned int address)
//{
//	unsigned int result;
//	uint32_t temp_address = address;
//	result = read_memory_8(temp_address);
//	temp_address++;
//	result = read_memory_8(temp_address) | (result << 8);
//	return result;
//}

void E64::mmu_ic::write_memory_8(unsigned int address, unsigned int value)
{
	uint32_t page = address >> 8;
	
	if (page == IO_VICV_PAGE) {
		machine.vicv->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_SND_PAGE) {
		machine.sids->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_TIMER_PAGE) {
		machine.timer->write_byte(address & 0xff, value & 0xff);
	} else if (page == IO_CIA_PAGE) {
		machine.cia->write_byte(address & 0xff, value & 0xff);
	} else {
		ram[address & 0xffffff] = value & 0xff;
	}
}

//void E64::mmu_ic::write_memory_16(unsigned int address, unsigned int value)
//{
//	uint32_t temp_address = address;
//	write_memory_8(temp_address, value >> 8);
//	temp_address++;
//	write_memory_8(temp_address, value & 0xff);
//}



void E64::mmu_ic::allocation_init()
{
	block_list = (struct block *)heap_start;

	// set the first block
	block_list->size = ((uint64_t)heap_end - (uint64_t)heap_start) -
		sizeof(struct block);
	block_list->free = 1;
	block_list->next = NULL;
	
	printf("[mmu allocation] available to E64-SQ: %lu kb\n",
	       (block_list->size)/1024);
}

void E64::mmu_ic::split(struct block *fitting_slot, size_t size)
{
	struct block *new_block = (struct block *)((size_t)fitting_slot +
		size + sizeof(struct block));

	new_block->size = (fitting_slot->size) - size - sizeof(struct block);
	new_block->free = 1;
	new_block->next = fitting_slot->next;

	fitting_slot->size = size;
	fitting_slot->free = 0;
	fitting_slot->next = new_block;
}

void *E64::mmu_ic::malloc(size_t bytes)
{
	// ensure even amount of bytes
	if (bytes & 0b1) bytes++;

	struct block *curr, *prev;
	void *result;

	curr = block_list;

	while ((((curr->size) < bytes) || ((curr->free) == 0)) && ((curr->next) != NULL)) {
		prev = curr;
		curr = curr->next;
	}

	if ((curr->size) == bytes) {
		curr->free = 0;
		result = (void *)++curr; // points to memory straight after struct
		// exact fitting of required memory
		return result;
	} else if ((curr->size) > (bytes + sizeof(struct block))) {
		split(curr, bytes);
		result = (void *)++curr;
		// allocation with a split
		return result;
	} else {
		result = NULL;
		// no memory available from heap
		return result;
	}
}

void E64::mmu_ic::merge()
{
	struct block *curr, *prev;
	curr = block_list;
	while (curr && (curr->next)) {
		if ((curr->free) && (curr->next->free)) {
			curr->size += (curr->next->size) + sizeof(struct block);
			curr->next = curr->next->next;
		}
		prev = curr;
		curr = curr->next;
	}
}

void E64::mmu_ic::free(void *ptr)
{
	if ((((void *)block_list) <= ptr) && (ptr <= ((void *)heap_end))) {
		struct block *curr = (struct block *)ptr;
		--curr;
		curr->free = 1;
		merge();
	}
}

size_t E64::mmu_ic::get_size(void *ptr)
{
	struct block *curr = (struct block *)ptr;
	--curr;
	return curr->size;
}

void *E64::mmu_ic::realloc(void *ptr, size_t size)
{
	if (ptr == NULL) {
		return malloc(size);
	}

	size_t old_size = get_size(ptr);

	if (size < old_size)
		return ptr;

	void *new_ptr = malloc(size);

	if (new_ptr == NULL) {
		return NULL;
	}

	memcpy(new_ptr, ptr, old_size);
	free(ptr);
	return new_ptr;
}
