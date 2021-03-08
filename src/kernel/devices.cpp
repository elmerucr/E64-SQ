#include "devices.hpp"
#include "common.hpp"

void E64::devices_t::cia_set_keyboard_repeat_delay(uint8_t delay)
{
	machine.cia->write_byte(0x02, delay);
}

void E64::devices_t::cia_set_keyboard_repeat_speed(uint8_t speed)
{
	machine.cia->write_byte(0x03, speed);
}

void E64::devices_t::cia_generate_key_events()
{
	machine.cia->write_byte(0x01, 0b00000001);
}

void E64::devices_t::timer_set(uint8_t timer_no, uint16_t bpm)
{
	timer_no &= 0b111; // confine to 0 - 7
	
	machine.timer->write_byte(0x02, (bpm & 0xff00) >> 8);
	machine.timer->write_byte(0x03, bpm & 0xff);
	
	machine.timer->write_byte(0x01, 0b1 << timer_no);
}
