#ifndef DEVICES_HPP
#define DEVICES_HPP

#include <cstdint>

namespace E64 {

class devices_t {
public:
	void cia_set_keyboard_repeat_delay(uint8_t delay);
	void cia_set_keyboard_repeat_speed(uint8_t speed);
	void cia_generate_key_events();
	
	void timer_set(uint8_t timer_no, uint16_t bpm);
};

}

#endif
