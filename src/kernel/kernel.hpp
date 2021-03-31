#include <cstdlib>
#include <cstdint>

#include "lua.hpp"
#include "blitter.hpp"
#include "cia.hpp"
#include "tty.hpp"
#include "devices.hpp"

#ifndef KERNEL_HPP
#define KERNEL_HPP

#define MAXINPUT 1024

namespace E64 {

enum overhead_state_t {
	OVERHEAD_NOT_VISIBLE,
	OVERHEAD_VISIBLE
};

class kernel_t {
private:
	devices_t devices;
public:
	kernel_t();
	~kernel_t();
	
	blitter_ic *blitter;
	cia_ic *cia;
	
	lua_State *L;
	
	tty_t *terminal;
	tty_t *stats_view;
	tty_t *cpu_view;
	tty_t *disassembly_view;
	tty_t *stack_view;
	tty_t *bar_1_height;
	tty_t *bar_2_height;
	
	bool stats_visible;
	enum overhead_state_t overhead_state;
	bool overhead_visible;
	
	void reset();
	void execute();
	void process_keypress();
	
	// events
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
