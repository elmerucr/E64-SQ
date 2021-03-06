#include <cstdlib>
#include <cstdint>

#include "squirrel.h"
#include "blitter.hpp"
#include "tty.hpp"

#ifndef KERNEL_HPP
#define KERNEL_HPP

namespace E64 {

class kernel_t {
private:
	HSQUIRRELVM v;
	
	surface_t *probeersel;
	int16_t xje;
	
	void init_chars();
public:
	kernel_t();
	~kernel_t();
	
	tty_t *tty;
	uint16_t *chars;
	
	void reset();
	void vblank_event();
	void timer_event();
	void execute();
};

}

#endif
