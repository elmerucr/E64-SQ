/*
 * mmu.hpp
 * E64-II
 *
 * Copyright © 2019-2020 elmerucr. All rights reserved.
*/

#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include <cstdlib>

#define IO_CIA_PAGE		0xfb03
#define IO_VICV_PAGE		0xfb04
#define IO_SND_PAGE		0xfb05
#define IO_TIMER_PAGE		0xfb06

namespace E64
{

struct block {
	size_t size;		// must be evenly sized
	uint16_t free;		// 0 = occupied, 1 = free
	struct block *next;
};

class mmu_ic
{
public:
	mmu_ic();
	~mmu_ic();
	
	uint8_t  *ram;          // make this private and work with friend class?
	uint16_t *ram_as_words; // make this private and work with friend class?
	
	void reset();
	
	unsigned int read_memory_8(unsigned int address);
//	unsigned int read_memory_16(unsigned int address);
	
	void write_memory_8(unsigned int address, unsigned int value);
//	void write_memory_16(unsigned int address, unsigned int value);
	

	// memory allocation things
 private:
	 void *heap_start;
	 void *heap_end;
	 struct block *block_list;
	 void split(struct block *fitting_slot, size_t size);
	 void merge();
	 size_t get_size(void *ptr);
 public:
	 void allocation_init();
	 void *malloc(size_t bytes);
	 void *realloc(void *ptr, size_t size);
	 void free(void *ptr);
};

}

#endif
