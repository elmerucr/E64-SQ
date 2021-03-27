//  stats.hpp
//  E64-SQ
//
//  Copyright © 2020-2021 elmerucr. All rights reserved.

#include <cstdint>
#include <chrono>

#ifndef STATS_HPP
#define STATS_HPP

namespace E64
{

class stats_t
{
private:
    std::chrono::time_point<std::chrono::steady_clock> now, then, done;
    int64_t total_time;
    int64_t total_idle_time;

    uint8_t framecounter;               // keeps track of no of frames since last evaluation
    uint8_t framecounter_interval;      // amount of frames between two evaluations

    uint8_t status_bar_framecounter;    // the status bar on the bottom is refreshed every few frames
    uint8_t status_bar_framecounter_interval;
    
    double alpha;                       // exponential smoothing constant

    double framerate;
    double smoothed_framerate;
    
    double mhz;
    double smoothed_mhz;
    
    double audio_queue_size;
    double smoothed_audio_queue_size;
    
    double idle_per_frame;
    double smoothed_idle_per_frame;
    
    char statistics_string[256];
    
public:
    void reset();
    
    uint32_t frametime;      // in microseconds
    
    // process calculations on parameters (fps/mhz/buffersize)
    void process_parameters();

    // for time measurement within a frame
    void start_idle_time();
    void end_idle_time();

    inline double current_framerate() { return framerate; }
    inline double current_smoothed_framerate() { return smoothed_framerate; }
    inline double current_audio_queue_size() { return audio_queue_size; }
    inline double current_smoothed_audio_queue_size() { return smoothed_audio_queue_size; }
    inline char *summary() { return statistics_string; }
};

}

#endif
