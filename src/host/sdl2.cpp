//  sdl2.cpp
//  E64
//
//  Copyright © 2017-2021 elmerucr. All rights reserved.

#include <cstdio>
#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include "common.hpp"
#include "sdl2.hpp"

SDL_AudioDeviceID E64_sdl2_audio_dev;
SDL_AudioSpec want, have;
bool audio_running;

const uint8_t *E64_sdl2_keyboard_state;

void E64::sdl2_init()
{
    SDL_Init(SDL_INIT_AUDIO);
    
    // each call to SDL_PollEvent invokes SDL_PumpEvents() that updates this array
    E64_sdl2_keyboard_state = SDL_GetKeyboardState(NULL);

    // INIT AUDIO STUFF
    // print the list of audio backends
    int numAudioDrivers = SDL_GetNumAudioDrivers();
    printf("[SDL] %d audio backend(s) compiled into SDL: ", numAudioDrivers);
    for(int i=0; i<numAudioDrivers; i++)
    {
        printf(" \'%s\' ", SDL_GetAudioDriver(i));
    }
    printf("\n");
    // What's this all about???
    SDL_zero(want);
    // audio specification
    want.freq = SAMPLE_RATE;
    // signed 16bit, little endian
    want.format = AUDIO_S16LSB;
    // 1, 2, 4, or 6  -  1 is mono, 2 is stereo...
    want.channels = 2;
    // power of 2, or 0 then env SDL_AUDIO_SAMPLES is used
    want.samples = 512;
    // can't be NULL
    want.callback = NULL;
    /* open audio device, allowing any changes to the specification */
    E64_sdl2_audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(!E64_sdl2_audio_dev)
    {
        printf("[SDL] failed to open audio device: %s\n", SDL_GetError());
        // this is not enough and even wrong...
        // consider a system without audio support and a frame delay based on FPS?
        SDL_Quit();
    }
    printf("[SDL] now using backend '%s'\n", SDL_GetCurrentAudioDriver());
    printf("[SDL] audio information\n");
    printf("\t\t\twant\thave\n");
    printf("frequency\t%d\t%d\n",want.freq,have.freq);
    printf("format\n float\t\t%s\t\t%s\n",SDL_AUDIO_ISFLOAT(want.format) ? "yes" : "no" ,SDL_AUDIO_ISFLOAT(have.format) ? "yes" : "no");
    printf(" signed\t\t%s\t\t%s\n", SDL_AUDIO_ISSIGNED(want.format) ? "yes" : "no", SDL_AUDIO_ISSIGNED(have.format) ? "yes" : "no");
    printf(" big endian\t%s\t\t%s\n", SDL_AUDIO_ISBIGENDIAN(want.format) ? "yes" : "no", SDL_AUDIO_ISBIGENDIAN(have.format) ? "yes" : "no");
    printf(" bitsize\t%d\t\t%d\n", SDL_AUDIO_BITSIZE(want.format), SDL_AUDIO_BITSIZE(have.format));
    printf("channels\t%d\t\t%d\n", want.channels, have.channels);
    printf("samples\t\t%d\t\t%d\n", want.samples, have.samples);
    audio_running = false;
}

int E64::sdl2_process_events()
{
    int return_value = NO_EVENT;
    
    SDL_Event event;

    //bool shift_pressed = E64_sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] | E64_sdl2_keyboard_state[SDL_SCANCODE_RSHIFT];
    bool alt_pressed   = E64_sdl2_keyboard_state[SDL_SCANCODE_LALT]   | E64_sdl2_keyboard_state[SDL_SCANCODE_RALT];
    //bool gui_pressed   = E64_sdl2_keyboard_state[SDL_SCANCODE_LGUI]   | E64_sdl2_keyboard_state[SDL_SCANCODE_RGUI];

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
                
                return_value = KEYPRESS_EVENT;          // default at keydown, may change to QUIT_EVENT
                

                if( (event.key.keysym.sym == SDLK_f) && alt_pressed )
                {
                    E64::sdl2_wait_until_f_released();
                    host.video->toggle_fullscreen();
                }
                else if( (event.key.keysym.sym == SDLK_r) && alt_pressed ) {
			E64::sdl2_wait_until_r_released();
			machine.reset();
			kernel->reset();
			stats.reset();
                }
                else if( (event.key.keysym.sym == SDLK_q) && alt_pressed )
                {
                    E64::sdl2_wait_until_q_released();
                    return_value = QUIT_EVENT;
                }
		else if(event.key.keysym.sym == SDLK_F9) {
			      kernel->overhead_visible = !kernel->overhead_visible;
		      }
                else if(event.key.keysym.sym == SDLK_F10)
                    {
			    kernel->stats_visible = !kernel->stats_visible;
                    }
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    printf("[SDL] window resize event\n");
                }
                break;
            case SDL_QUIT:
                return_value = QUIT_EVENT;
                break;
        }
    }

    // update keystates in cia chip, regardless if machine is in debug or running mode
    if( !alt_pressed )
    {
	    machine.cia->keys_last_known_state[SCANCODE_ESCAPE] = E64_sdl2_keyboard_state[SDL_SCANCODE_ESCAPE] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F1] = E64_sdl2_keyboard_state[SDL_SCANCODE_F1] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F2] = E64_sdl2_keyboard_state[SDL_SCANCODE_F2] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F3] = E64_sdl2_keyboard_state[SDL_SCANCODE_F3] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F4] = E64_sdl2_keyboard_state[SDL_SCANCODE_F4] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F5] = E64_sdl2_keyboard_state[SDL_SCANCODE_F5] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F6] = E64_sdl2_keyboard_state[SDL_SCANCODE_F6] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F7] = E64_sdl2_keyboard_state[SDL_SCANCODE_F7] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F8] = E64_sdl2_keyboard_state[SDL_SCANCODE_F8] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_GRAVE] = (machine.cia->registers[SCANCODE_GRAVE] << 1) | (E64_sdl2_keyboard_state[SDL_SCANCODE_GRAVE] ? 0x01 : 0x00);
	    machine.cia->keys_last_known_state[SCANCODE_1] = E64_sdl2_keyboard_state[SDL_SCANCODE_1] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_2] = E64_sdl2_keyboard_state[SDL_SCANCODE_2] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_3] = E64_sdl2_keyboard_state[SDL_SCANCODE_3] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_4] = E64_sdl2_keyboard_state[SDL_SCANCODE_4] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_5] = E64_sdl2_keyboard_state[SDL_SCANCODE_5] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_6] = E64_sdl2_keyboard_state[SDL_SCANCODE_6] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_7] = E64_sdl2_keyboard_state[SDL_SCANCODE_7] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_8] = E64_sdl2_keyboard_state[SDL_SCANCODE_8] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_9] = E64_sdl2_keyboard_state[SDL_SCANCODE_9] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_0] = E64_sdl2_keyboard_state[SDL_SCANCODE_0] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_MINUS] = E64_sdl2_keyboard_state[SDL_SCANCODE_MINUS] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_EQUALS] = E64_sdl2_keyboard_state[SDL_SCANCODE_EQUALS] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_BACKSPACE] = E64_sdl2_keyboard_state[SDL_SCANCODE_BACKSPACE] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_TAB] = E64_sdl2_keyboard_state[SDL_SCANCODE_TAB] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_Q] = E64_sdl2_keyboard_state[SDL_SCANCODE_Q] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_W] = E64_sdl2_keyboard_state[SDL_SCANCODE_W] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_E] = E64_sdl2_keyboard_state[SDL_SCANCODE_E] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_R] = E64_sdl2_keyboard_state[SDL_SCANCODE_R] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_T] = E64_sdl2_keyboard_state[SDL_SCANCODE_T] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_Y] = E64_sdl2_keyboard_state[SDL_SCANCODE_Y] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_U] = E64_sdl2_keyboard_state[SDL_SCANCODE_U] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_I] = E64_sdl2_keyboard_state[SDL_SCANCODE_I] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_O] = E64_sdl2_keyboard_state[SDL_SCANCODE_O] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_P] = E64_sdl2_keyboard_state[SDL_SCANCODE_P] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_LEFTBRACKET] = E64_sdl2_keyboard_state[SDL_SCANCODE_LEFTBRACKET] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_RIGHTBRACKET] = E64_sdl2_keyboard_state[SDL_SCANCODE_RIGHTBRACKET] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_RETURN] = E64_sdl2_keyboard_state[SDL_SCANCODE_RETURN] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_A] = E64_sdl2_keyboard_state[SDL_SCANCODE_A] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_S] = E64_sdl2_keyboard_state[SDL_SCANCODE_S] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_D] = E64_sdl2_keyboard_state[SDL_SCANCODE_D] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_F] = E64_sdl2_keyboard_state[SDL_SCANCODE_F] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_G] = E64_sdl2_keyboard_state[SDL_SCANCODE_G] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_H] = E64_sdl2_keyboard_state[SDL_SCANCODE_H] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_J] = E64_sdl2_keyboard_state[SDL_SCANCODE_J] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_K] = E64_sdl2_keyboard_state[SDL_SCANCODE_K] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_L] = E64_sdl2_keyboard_state[SDL_SCANCODE_L] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_SEMICOLON] = E64_sdl2_keyboard_state[SDL_SCANCODE_SEMICOLON] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_APOSTROPHE] = E64_sdl2_keyboard_state[SDL_SCANCODE_APOSTROPHE] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_BACKSLASH] = E64_sdl2_keyboard_state[SDL_SCANCODE_BACKSLASH] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_LSHIFT] = E64_sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_Z] = E64_sdl2_keyboard_state[SDL_SCANCODE_Z] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_X] = E64_sdl2_keyboard_state[SDL_SCANCODE_X] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_C] = E64_sdl2_keyboard_state[SDL_SCANCODE_C] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_V] = E64_sdl2_keyboard_state[SDL_SCANCODE_V] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_B] = E64_sdl2_keyboard_state[SDL_SCANCODE_B] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_N] = E64_sdl2_keyboard_state[SDL_SCANCODE_N] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_M] = E64_sdl2_keyboard_state[SDL_SCANCODE_M] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_COMMA] = E64_sdl2_keyboard_state[SDL_SCANCODE_COMMA] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_PERIOD] = E64_sdl2_keyboard_state[SDL_SCANCODE_PERIOD] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_SLASH] = E64_sdl2_keyboard_state[SDL_SCANCODE_SLASH] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_RSHIFT] = E64_sdl2_keyboard_state[SDL_SCANCODE_RSHIFT] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_LCTRL] = E64_sdl2_keyboard_state[SDL_SCANCODE_LCTRL] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_SPACE] = E64_sdl2_keyboard_state[SDL_SCANCODE_SPACE] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_RCTRL] = E64_sdl2_keyboard_state[SDL_SCANCODE_RCTRL] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_LEFT] = E64_sdl2_keyboard_state[SDL_SCANCODE_LEFT] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_UP] = E64_sdl2_keyboard_state[SDL_SCANCODE_UP] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_DOWN] = E64_sdl2_keyboard_state[SDL_SCANCODE_DOWN] ? 0x01 : 0x00;
	    machine.cia->keys_last_known_state[SCANCODE_RIGHT] = E64_sdl2_keyboard_state[SDL_SCANCODE_RIGHT] ? 0x01 : 0x00;
	}
	if (return_value == QUIT_EVENT)
		printf("[SDL] detected quit event\n");
	return return_value;
}

void E64::sdl2_wait_until_enter_released()
{
	SDL_Event event;
	bool wait = true;
	while (wait) {
		SDL_PollEvent(&event);
		if ((event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_RETURN))
			wait = false;
		std::this_thread::sleep_for(std::chrono::microseconds(40000));
	}
}

void E64::sdl2_wait_until_f9_released()
{
    SDL_Event event;
    bool wait = true;
    while(wait) {
        SDL_PollEvent(&event);
        if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_F9) ) wait = false;
        std::this_thread::sleep_for(std::chrono::microseconds(40000));
    }
}

void E64::sdl2_wait_until_f_released()
{
    SDL_Event event;
    bool wait = true;
    while(wait) {
        SDL_PollEvent(&event);
        if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_f) ) wait = false;
        std::this_thread::sleep_for(std::chrono::microseconds(40000));
    }
}


void E64::sdl2_wait_until_q_released()
{
    SDL_Event event;
    bool wait = true;
    while(wait) {
        SDL_PollEvent(&event);
        if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_q) ) wait = false;
        std::this_thread::sleep_for(std::chrono::microseconds(40000));
    }
}


void E64::sdl2_wait_until_r_released()
{
    SDL_Event event;
    bool wait = true;
    while(wait) {
        SDL_PollEvent(&event);
        if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == SDLK_r) ) wait = false;
        std::this_thread::sleep_for(std::chrono::microseconds(40000));
    }
}


void E64::sdl2_queue_audio(void *buffer, unsigned size)
{
    SDL_QueueAudio(E64_sdl2_audio_dev, buffer, size);
}

unsigned int E64::sdl2_get_queued_audio_size()
{
    return SDL_GetQueuedAudioSize(E64_sdl2_audio_dev);
}

void E64::sdl2_start_audio()
{
    if(!audio_running)
    {
        printf("[SDL] start audio\n");
        // Unpause audiodevice, and process audiostream
        SDL_PauseAudioDevice(E64_sdl2_audio_dev, 0);
        audio_running = true;
    }
}

void E64::sdl2_stop_audio()
{
    if(audio_running)
    {
        printf("[SDL] stop audio\n");
        // Pause audiodevice
        SDL_PauseAudioDevice(E64_sdl2_audio_dev, 1);
        audio_running = false;
    }
}

void E64::sdl2_cleanup()
{
    printf("[SDL] cleaning up\n");
    E64::sdl2_stop_audio();
    SDL_CloseAudioDevice(E64_sdl2_audio_dev);
    //SDL_Quit();
}
