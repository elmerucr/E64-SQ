#include <cstdlib>
#include <cstdint>

#ifndef KERNEL_HPP
#define KERNEL_HPP

namespace E64 {

struct block {
	size_t size;		// must be evenly sized
	uint16_t free;		// 0 = occupied, 1 = free
	struct block *next;
};

class kernel_t {
private:
	void *heap_start;
	void *heap_end;
public:
	kernel_t();
	void reset();
	void vblank_event();
	void timer_event();
	void run();
};

}

#endif
