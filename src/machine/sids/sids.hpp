//  sids.hpp
//  E64
//
//  Copyright © 2019 elmerucr. All rights reserved.

#include <cstdio>
#include <cstdint>

// resid header
#include "sid.h"

#ifndef sids_hpp
#define sids_hpp

namespace E64
{
    class sids_ic
    {
        SID sid[2];
        uint8_t balance_registers[8];
        cycle_count delta_t_sid0;
        int16_t sample_buffer_mono_sid0[65536];
        cycle_count delta_t_sid1;
        int16_t sample_buffer_mono_sid1[65536];
        int16_t sample_buffer_stereo[131072];
    public:
        sids_ic();
        ~sids_ic();
        // read and write functions to data registers of sid array and mixer
        uint8_t read_byte(uint8_t address);
        void write_byte(uint8_t address, uint8_t byte);
        // run the no of cycles that need to be processed by the sid chips on the sound device
        // and process all the accumulated cycles (flush into soundbuffer)
        void run(uint32_t number_of_cycles);
        void reset();
    };
}

#endif
