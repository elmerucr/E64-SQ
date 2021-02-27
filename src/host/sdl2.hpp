//  sdl2.hpp
//  E64-SQ
//
//  Copyright © 2017-2021 elmerucr. All rights reserved.

#ifndef sdl2_hpp
#define sdl2_hpp

namespace E64
{
    enum events_output_state
    {
        QUIT_EVENT = -1,
        NO_EVENT = 0,
        KEYPRESS_EVENT = 1
    };

    // general init and cleanup
    void sdl2_init();
    void sdl2_cleanup();

    // event related
    int sdl2_process_events();
    void sdl2_wait_until_enter_released();
    void sdl2_wait_until_f_released();
    void sdl2_wait_until_q_released();
    void sdl2_wait_until_f9_released();
    void sdl2_wait_until_r_released();

    // audio related
    void sdl2_start_audio();
    void sdl2_stop_audio();
    void sdl2_queue_audio(void *buffer, unsigned size);
    unsigned int sdl2_get_queued_audio_size();
}

#endif
