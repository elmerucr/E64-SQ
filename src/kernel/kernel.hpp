#include <cstdlib>
#include <cstdint>

#include "allocation.hpp"
#include "squirrel.h"

#ifndef KERNEL_HPP
#define KERNEL_HPP

namespace E64 {

class kernel_t {
private:
	allocation *memory_manager;
	HSQUIRRELVM v;
public:
	kernel_t();
	~kernel_t();
	
	void reset();
	void vblank_event();
	void timer_event();
	void run();
};

}

#endif
