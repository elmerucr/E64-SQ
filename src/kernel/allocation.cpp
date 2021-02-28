#include <cstdio>
#include <cstring>
#include "allocation.hpp"

E64::allocation::allocation(void *start, void *end)
{
	heap_start = start;
	heap_end = end;
	
	printf("[kernel memory allocation] size of memblock meta info: %lu\n", sizeof(struct block));
	init();
}

void E64::allocation::init()
{
	block_list = (struct block *)heap_start;

	// set the first block
	block_list->size = ((uint64_t)heap_end - (uint64_t)heap_start) - sizeof(struct block);
	block_list->free = 1;
	block_list->next = NULL;
	
	printf("[kernel memory allocation] available to E64-SQ: %lu kb\n", (block_list->size)/1024);
}

void E64::allocation::split(struct block *fitting_slot, size_t size)
{
	struct block *new_block = (struct block *)((size_t)fitting_slot + size + sizeof(struct block));

	new_block->size = (fitting_slot->size) - size - sizeof(struct block);
	new_block->free = 1;
	new_block->next = fitting_slot->next;

	fitting_slot->size = size;
	fitting_slot->free = 0;
	fitting_slot->next = new_block;
}

void *E64::allocation::malloc(size_t bytes)
{
	// ensure even amount of bytes
	if (bytes & 0b1) bytes++;

	struct block *curr, *prev;
	void *result;

	curr = block_list;

	while (((curr->size) < bytes) || ((curr->free) == 0) && ((curr->next) != NULL)) {
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

void E64::allocation::merge()
{
	struct block *curr, *prev;
	curr = block_list;
	while ((curr->next) != NULL) {
		if ((curr->free) && (curr->next->free)) {
			curr->size += (curr->next->size) + sizeof(struct block);
			curr->next = curr->next->next;
		}
		prev = curr;
		curr = curr->next;
	}
}

void E64::allocation::free(void *ptr)
{
	// NEEDS WORK
	// this is a very minimal implementation
	// doesn't iterate through the blocks to check if the given pointer is valid
	if ( (((void *)block_list) <= ptr) && (ptr <= ((void *)heap_end))) {
		struct block *curr = (struct block *)ptr;
		--curr;
		curr->free = 1;
		merge();
	}
}

size_t E64::allocation::get_size(void *ptr)
{
	struct block *curr = (struct block *)ptr;
	--curr;
	return curr->size;
}

void *E64::allocation::realloc(void *ptr, size_t size)
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
