#include <cstdlib>
#include <cstdint>

#include "lua.hpp"
#include "blitter.hpp"
#include "cia.hpp"
#include "tty.hpp"
#include "timer.hpp"

#ifndef HUD_HPP
#define HUD_HPP

#define MAXINPUT 1024

namespace E64 {

class hud_t {
private:
	bool irq_line;
	
	void process_command(char *buffer);
public:
	hud_t();
	~hud_t();
	
	bool paused;
	
	void flip_modes();

	void memory_dump(uint16_t address, int rows);
	void enter_monitor_line(char *buffer);
	bool hex_string_to_int(const char *temp_string, uint16_t *return_value);
	
	blitter_ic *blitter;
	cia_ic *cia;
	timer_ic *timer;
	
	lua_State *L;
	
	tty_t *terminal;
	tty_t *stats_view;
	tty_t *cpu_view;
	tty_t *disassembly_view;
	tty_t *stack_view;
	tty_t *bar_single_height_small_1;
	tty_t *bar_single_height_small_2;
	tty_t *bar_single_height;
	tty_t *bar_double_height;
	tty_t *other_info;
	
	bool stats_visible;
	
	void reset();
	void run(uint16_t cycles);
	void update();
	void update_stats_view();
	void process_keypress();
	void redraw();
	
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
