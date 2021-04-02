//  cia.cpp
//  E64
//
//  Copyright © 2019-2020 elmerucr. All rights reserved.

#include "cia.hpp"
#include "common.hpp"
#include "sdl2.hpp"
#include <cstdio>

bool scancode_not_modifier[] =
{
    true,                       // 0x00
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x08
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x10
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x18
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x20
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x28
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x30
    true,
    false,                      // left shift
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x38
    true,
    true,
    true,
    true,
    false,                      // right shift
    false,                      // left control
    //false,                      // left alt
    true,
    //false,                      // right alt
    false,                      // right control
    true,
    true,
    true,
    true
};

#define SHIFT_PRESSED   0b00000001
#define CTRL_PRESSED    0b00000010
#define ALT_PRESSED     0b00000100

inline char event_to_ascii(uint8_t scancode, uint8_t modifiers)
{
    switch (scancode)
    {
        case E64::SCANCODE_ESCAPE:
            return ASCII_ESCAPE;
        case E64::SCANCODE_F1:
            return ASCII_F1;
        case E64::SCANCODE_F2:
            return ASCII_F2;
        case E64::SCANCODE_F3:
            return ASCII_F3;
        case E64::SCANCODE_F4:
            return ASCII_F4;
        case E64::SCANCODE_F5:
            return ASCII_F5;
        case E64::SCANCODE_F6:
            return ASCII_F6;
        case E64::SCANCODE_F7:
            return ASCII_F7;
        case E64::SCANCODE_F8:
            return ASCII_F8;
        case E64::SCANCODE_GRAVE:
            return (modifiers & SHIFT_PRESSED) ? ASCII_TILDE : ASCII_GRAVE;
        case E64::SCANCODE_1:
            return (modifiers & SHIFT_PRESSED) ? ASCII_EXCL_MARK : ASCII_1;
        case E64::SCANCODE_2:
            return (modifiers & SHIFT_PRESSED) ? ASCII_AT : ASCII_2;
        case E64::SCANCODE_3:
            return (modifiers & SHIFT_PRESSED) ? ASCII_NUMBER : ASCII_3;
        case E64::SCANCODE_4:
            return (modifiers & SHIFT_PRESSED) ? ASCII_DOLLAR : ASCII_4;
        case E64::SCANCODE_5:
            return (modifiers & SHIFT_PRESSED) ? ASCII_PERCENT : ASCII_5;
        case E64::SCANCODE_6:
            return (modifiers & SHIFT_PRESSED) ? ASCII_CARET : ASCII_6;
        case E64::SCANCODE_7:
            return (modifiers & SHIFT_PRESSED) ? ASCII_AMPERSAND : ASCII_7;
        case E64::SCANCODE_8:
            return (modifiers & SHIFT_PRESSED) ? ASCII_ASTERISK : ASCII_8;
        case E64::SCANCODE_9:
            return (modifiers & SHIFT_PRESSED) ? ASCII_OPEN_PAR : ((modifiers & CTRL_PRESSED) ? ASCII_REVERSE_ON : ASCII_9);
        case E64::SCANCODE_0:
            return (modifiers & SHIFT_PRESSED) ? ASCII_CLOSE_PAR : ((modifiers & CTRL_PRESSED) ? ASCII_REVERSE_OFF : ASCII_0);
        case E64::SCANCODE_MINUS:
            return (modifiers & SHIFT_PRESSED) ? ASCII_UNDERSCORE : ASCII_HYPHEN;
        case E64::SCANCODE_EQUALS:
            return (modifiers & SHIFT_PRESSED) ? ASCII_PLUS : ASCII_EQUALS;
        case E64::SCANCODE_BACKSPACE:
            return ASCII_BACKSPACE;
        case E64::SCANCODE_TAB:
            return ASCII_HOR_TAB;
        case E64::SCANCODE_Q:
            return (modifiers & SHIFT_PRESSED) ? ASCII_Q : ASCII_q;
        case E64::SCANCODE_W:
            return (modifiers & SHIFT_PRESSED) ? ASCII_W : ASCII_w;
        case E64::SCANCODE_E:
            return (modifiers & SHIFT_PRESSED) ? ASCII_E : ASCII_e;
        case E64::SCANCODE_R:
            return (modifiers & SHIFT_PRESSED) ? ASCII_R : ASCII_r;
        case E64::SCANCODE_T:
            return (modifiers & SHIFT_PRESSED) ? ASCII_T : ASCII_t;
        case E64::SCANCODE_Y:
            return (modifiers & SHIFT_PRESSED) ? ASCII_Y : ASCII_y;
        case E64::SCANCODE_U:
            return (modifiers & SHIFT_PRESSED) ? ASCII_U : ASCII_u;
        case E64::SCANCODE_I:
            return (modifiers & SHIFT_PRESSED) ? ASCII_I : ASCII_i;
        case E64::SCANCODE_O:
            return (modifiers & SHIFT_PRESSED) ? ASCII_O : ASCII_o;
        case E64::SCANCODE_P:
            return (modifiers & SHIFT_PRESSED) ? ASCII_P : ASCII_p;
        case E64::SCANCODE_LEFTBRACKET:
            return (modifiers & SHIFT_PRESSED) ? ASCII_OPEN_BRACE : ASCII_OPEN_BRACK;
        case E64::SCANCODE_RIGHTBRACKET:
            return (modifiers & SHIFT_PRESSED) ? ASCII_CLOSE_BRACE : ASCII_CLOSE_BRACK;
        case E64::SCANCODE_RETURN:
            return ASCII_LF;
        case E64::SCANCODE_A:
            return (modifiers & SHIFT_PRESSED) ? ASCII_A : ASCII_a;
        case E64::SCANCODE_S:
            return (modifiers & SHIFT_PRESSED) ? ASCII_S : ASCII_s;
        case E64::SCANCODE_D:
            return (modifiers & SHIFT_PRESSED) ? ASCII_D : ASCII_d;
        case E64::SCANCODE_F:
            return (modifiers & SHIFT_PRESSED) ? ASCII_F : ASCII_f;
        case E64::SCANCODE_G:
            return (modifiers & SHIFT_PRESSED) ? ASCII_G : ASCII_g;
        case E64::SCANCODE_H:
            return (modifiers & SHIFT_PRESSED) ? ASCII_H : ASCII_h;
        case E64::SCANCODE_J:
            return (modifiers & SHIFT_PRESSED) ? ASCII_J : ASCII_j;
        case E64::SCANCODE_K:
            return (modifiers & SHIFT_PRESSED) ? ASCII_K : ASCII_k;
        case E64::SCANCODE_L:
            return (modifiers & SHIFT_PRESSED) ? ASCII_L : ASCII_l;
        case E64::SCANCODE_SEMICOLON:
            return (modifiers & SHIFT_PRESSED) ? ASCII_COLON : ASCII_SEMI_COLON;
        case E64::SCANCODE_APOSTROPHE:
            return (modifiers & SHIFT_PRESSED) ? ASCII_DOUBLE_QUOTES : ASCII_SINGLE_QUOTE;
        case E64::SCANCODE_BACKSLASH:
            return (modifiers & SHIFT_PRESSED) ? ASCII_VERT_BAR : ASCII_BACKSLASH;
        case E64::SCANCODE_Z:
            return (modifiers & SHIFT_PRESSED) ? ASCII_Z : ASCII_z;
        case E64::SCANCODE_X:
            return (modifiers & SHIFT_PRESSED) ? ASCII_X : ASCII_x;
        case E64::SCANCODE_C:
            return (modifiers & SHIFT_PRESSED) ? ASCII_C : ASCII_c;
        case E64::SCANCODE_V:
            return (modifiers & SHIFT_PRESSED) ? ASCII_V : ASCII_v;
        case E64::SCANCODE_B:
            return (modifiers & SHIFT_PRESSED) ? ASCII_B : ASCII_b;
        case E64::SCANCODE_N:
            return (modifiers & SHIFT_PRESSED) ? ASCII_N : ASCII_n;
        case E64::SCANCODE_M:
            return (modifiers & SHIFT_PRESSED) ? ASCII_M : ASCII_m;
        case E64::SCANCODE_COMMA:
            return (modifiers & SHIFT_PRESSED) ? ASCII_LESS : ASCII_COMMA;
        case E64::SCANCODE_PERIOD:
            return (modifiers & SHIFT_PRESSED) ? ASCII_GREATER : ASCII_PERIOD;
        case E64::SCANCODE_SLASH:
            return (modifiers & SHIFT_PRESSED) ? ASCII_QUESTION_M : ASCII_SLASH;
        case E64::SCANCODE_SPACE:
            return ASCII_SPACE;
        case E64::SCANCODE_LEFT:
            return ASCII_CURSOR_LEFT;
        case E64::SCANCODE_UP:
            return ASCII_CURSOR_UP;
        case E64::SCANCODE_DOWN:
            return ASCII_CURSOR_DOWN;
        case E64::SCANCODE_RIGHT:
            return ASCII_CURSOR_RIGHT;
    }
    return 0;
}

E64::cia_ic::cia_ic()
{
    cycles_per_interval = VICV_DOT_CLOCK_SPEED / 100; // no of cycles @ vicv clockspeed for a total of 10 ms
    reset();
}

void E64::cia_ic::reset()
{
    cycle_counter = 0;
    
    for(int i=0; i<256; i++) registers[i] = 0x00;
    //for(int i=0; i<128; i++) sdl2_keys_last_known_state[i] = 0x00;
    for(int i=0; i<256; i++) event_list[i] = 0x00;
    head = tail = 0;
    
    generating_key_events = false;
    
    key_down = false;
    keyboard_repeat_delay = 50;
    keyboard_repeat_speed = 5;
    keyboard_repeat_counter = 0;
}

void E64::cia_ic::push_event(uint8_t event)
{
    event_list[head] = event;
    head++;
    if( head == tail) tail++;
}

uint8_t E64::cia_ic::pop_event()
{
    uint8_t result;
    
    if( head == tail )
    {
        // no events in queue / stack return empty scancode
        result = E64::SCANCODE_EMPTY;
    }
    else
    {
        result = event_list[tail];
        tail++;
    }
    
    return result;
}

void E64::cia_ic::run(int no_of_cycles)
{
    cycle_counter += no_of_cycles;
    
    if( cycle_counter >= cycles_per_interval )
    {
        cycle_counter -= cycles_per_interval;
        
        // check modifier keys
        uint8_t modifier_keys_status =  (sdl2_keys_last_known_state[SCANCODE_LSHIFT] ? SHIFT_PRESSED : 0) |
                                        (sdl2_keys_last_known_state[SCANCODE_RSHIFT] ? SHIFT_PRESSED : 0) |
                                        (sdl2_keys_last_known_state[SCANCODE_LCTRL ] ? CTRL_PRESSED  : 0) |
                                        (sdl2_keys_last_known_state[SCANCODE_RCTRL ] ? CTRL_PRESSED  : 0);
        
        // registers 128 to 255 reflect the current keyboard state
        // shift each register one bit to the left, bit 0 is only set if key is pressed
        // if one of the keys changed its state, push an event
        for (int i=0x00; i<0x80; i++) {
            registers[0x80 | i] = (registers[0x80 | i] << 1) | sdl2_keys_last_known_state[i];

            switch (registers[0x80 | i] & 0b00000011) {
                case 0b01:
                    // Event: key pressed
                    if (generating_key_events && scancode_not_modifier[i]) {
                        key_down = true;
                        last_key = i;
                        keyboard_repeat_current_max = keyboard_repeat_delay;
                        keyboard_repeat_counter = 0;
                    }
                    break;
                case 0b10:
                    // Event: key released
                    if (generating_key_events) {
                        if (i == last_key)
				key_down = false;
                    }
                    break;
                default:
                    // do nothing
                    break;
            }
        }
        
        if (key_down) {
            if (keyboard_repeat_counter == 0)
		    push_event(event_to_ascii(last_key, modifier_keys_status));
            keyboard_repeat_counter++;
            if (keyboard_repeat_counter == keyboard_repeat_current_max) {
                keyboard_repeat_counter = 0;
                keyboard_repeat_current_max = keyboard_repeat_speed;
            }
        }
    }
}

uint8_t E64::cia_ic::read_byte(uint8_t address)
{
    uint8_t return_value = 0x00;
    
    switch (address) {
        case 0x00:
            return_value = events_waiting() ? 0x01 : 0x00;
            break;
        case 0x01:
            return_value = (generating_key_events ? 0x01 : 0x00);
            break;
        case 0x02:
            return_value = keyboard_repeat_delay;
            break;
        case 0x03:
            return_value = keyboard_repeat_speed;
            break;
        case 0x04:
            return_value = pop_event();
            break;
        default:
            return_value = registers[address];
            break;
    }
    
    return return_value;
}


void E64::cia_ic::write_byte(uint8_t address, uint8_t byte)
{
	switch (address) {
	case 0x01:
		if (byte & 0b00000001) {
			generating_key_events = true;
		} else {
			generating_key_events = false;
			key_down = false;
		}
		if (byte & 0b10000000) {
			// a write to bit 7 clears the event list
			tail = head;
			key_down = false;
		}
		break;
	case 0x02:
		keyboard_repeat_delay = byte;
		break;
	case 0x03:
		keyboard_repeat_speed = byte;
		break;
	default:
		// no other addresses are written to
		break;
	}
}

void E64::cia_ic::set_keyboard_repeat_delay(uint8_t delay)
{
	write_byte(0x02, delay);
}

void E64::cia_ic::set_keyboard_repeat_speed(uint8_t speed)
{
	write_byte(0x03, speed);
}

void E64::cia_ic::generate_key_events()
{
	write_byte(0x01, 0b00000001);
}
