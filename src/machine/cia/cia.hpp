//  cia.hpp
//  E64-II
//
//  Copyright © 2019-2020 elmerucr. All rights reserved.
//
//

/*  Register 0 - CIA Status Register
 *
 *  7 6 5 4 3 2 1 0
 *                |
 *                +-- No events waiting (0) / One or more keyboard events are waiting (1) (READ ONLY)
 *
 *  bits 1-7: Reserved
 *
 *
 *  Register 1 - CIA Control Register
 *
 *  7 6 5 4 3 2 1 0
 *  |             |
 *  |             +-- No keyboard events generated (0) / Keyboard events are generated (1) (READ/WRITE)
 *  +---------------- Write 1 clears the event list (WRITE ONLY)
 *
 *  bits 1-7: Reserved
 *
 *
 *  Register 2 - Keyboard Repeat Delay, 1 byte, in multiples of 10ms (READ/WRITE)
 *
 *  Register 3 - Keyboard Repeat Speed, 1 byte, in multiples of 10ms (READ/WRITE)
 *
 *  Register 4 - Contains next key ascii code (READ), returns 0 on events available
 *
 *  Register 0x80 -->  key state array
 *
 */

#include <cstdint>

#ifndef cia_hpp
#define cia_hpp

namespace E64
{

enum scancodes
{
    SCANCODE_EMPTY = 0x00,     // 0x00
    SCANCODE_ESCAPE,
    SCANCODE_F1,
    SCANCODE_F2,
    SCANCODE_F3,
    SCANCODE_F4,
    SCANCODE_F5,
    SCANCODE_F6,
    SCANCODE_F7,               // 0x08
    SCANCODE_F8,
    SCANCODE_GRAVE,
    SCANCODE_1,
    SCANCODE_2,
    SCANCODE_3,
    SCANCODE_4,
    SCANCODE_5,
    SCANCODE_6,                // 0x10
    SCANCODE_7,
    SCANCODE_8,
    SCANCODE_9,
    SCANCODE_0,
    SCANCODE_MINUS,
    SCANCODE_EQUALS,
    SCANCODE_BACKSPACE,
    SCANCODE_TAB,              // 0x18
    SCANCODE_Q,
    SCANCODE_W,
    SCANCODE_E,
    SCANCODE_R,
    SCANCODE_T,
    SCANCODE_Y,
    SCANCODE_U,
    SCANCODE_I,               // 0x20
    SCANCODE_O,
    SCANCODE_P,
    SCANCODE_LEFTBRACKET,
    SCANCODE_RIGHTBRACKET,
    SCANCODE_RETURN,
    SCANCODE_A,
    SCANCODE_S,
    SCANCODE_D,                // 0x28
    SCANCODE_F,
    SCANCODE_G,
    SCANCODE_H,
    SCANCODE_J,
    SCANCODE_K,
    SCANCODE_L,
    SCANCODE_SEMICOLON,
    SCANCODE_APOSTROPHE,       // 0x30
    SCANCODE_BACKSLASH,
    SCANCODE_LSHIFT,
    SCANCODE_Z,
    SCANCODE_X,
    SCANCODE_C,
    SCANCODE_V,
    SCANCODE_B,
    SCANCODE_N,                // 0x38
    SCANCODE_M,
    SCANCODE_COMMA,
    SCANCODE_PERIOD,
    SCANCODE_SLASH,
    SCANCODE_RSHIFT,
    SCANCODE_LCTRL,
    //SCANCODE_LALT,
    SCANCODE_SPACE,
    //SCANCODE_RALT,
    SCANCODE_RCTRL,              // 0x40
    SCANCODE_LEFT,
    SCANCODE_UP,
    SCANCODE_DOWN,
    SCANCODE_RIGHT
};

class cia_ic
{
private:
    uint32_t    cycle_counter;
    uint32_t    cycles_per_interval;
    
    void    push_event(uint8_t event);
    uint8_t pop_event();
    
    bool    generate_key_events;
    
    // implement a fifo event list, important for key presses, you don't want them in the wrong order
    uint8_t event_list[256];
    
    // 'head' always points to the currently available location for an event
    // if (head == tail), no events are available
    uint8_t head;
    uint8_t tail;
    
    bool    key_down;
    uint8_t last_key;
    uint8_t keyboard_repeat_delay;      // multiples of 10ms before keyboard starts repeating (60x = 0.6s)
    uint8_t keyboard_repeat_speed;      // multiples of 10ms between repeats (5x = 50ms -> 20Hz, or 4s to fill up screenline @ 80 columns)
    uint8_t keyboard_repeat_counter;
    uint8_t keyboard_repeat_current_max;
    
    inline bool events_waiting()
    {
        return (head == tail) ? false : true;
    }
    
public:
    cia_ic();
    
    // reset, also called by constructor
    void reset();
    
    // THESE NEXT ONES SHOULD BE PRIVATE IN THE FUTURE!
    uint8_t keys_last_known_state[128];
    uint8_t registers[256];
    //
    
    /*  Run a number of cycles */
    void run(int no_of_cycles);
    
    // register access functions
    uint8_t read_byte(uint8_t address);
    void write_byte(uint8_t address, uint8_t byte);
};

}

#endif
