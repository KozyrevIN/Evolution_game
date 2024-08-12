
#define fps_counter_header
#include "../include/fps_counter.h"

float FpsCounter::getFps() {
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    float fps = 1.0 / std::chrono::duration<float>(now - prev_time).count();
    prev_time = now;
    return fps;
}

FpsCounter::FpsCounter() {
    prev_time = std::chrono::high_resolution_clock::now();
}