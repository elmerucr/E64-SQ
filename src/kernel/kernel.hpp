#include <cstdlib>
#include <cstdint>

#include "lua.hpp"
#include "blitter.hpp"
#include "tty.hpp"
#include "devices.hpp"

#ifndef KERNEL_HPP
#define KERNEL_HPP

#define MAXINPUT 1024

namespace E64 {

class kernel_t {
private:
	devices_t devices;
public:
	kernel_t();
	~kernel_t();
	
	lua_State *L;
	
	tty_t *tty;
	tty_t *message_box;
	
	void reset();
	void execute();
	void process_keypress();
	
	// events
	void vblank_event();
	void timer_0_event();
	void timer_1_event();
	void timer_2_event();
	void timer_3_event();
	void timer_4_event();
	void timer_5_event();
	void timer_6_event();
	void timer_7_event();
};

}

#endif
