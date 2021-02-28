#ifndef ALLOCATION_HPP
#define ALLOCATION_HPP

#include <cstdint>
#include <cstdlib>

namespace E64 {

struct block {
	size_t size;		// must be evenly sized
	uint16_t free;		// 0 = occupied, 1 = free
	struct block *next;
};

class allocation {
private:
	void *heap_start;
	void *heap_end;
	
	struct block *block_list;
	
	void init();
	void split(struct block *fitting_slot, size_t size);
	void merge();
	size_t get_size(void *ptr);
public:
	allocation(void *start, void *end);
	void *malloc(size_t bytes);
	void *realloc(void *ptr, size_t size);
	void free(void *ptr);
};

}

#endif
