#include <chrono>

class FpsCounter {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;

public:
    float getFps();
    FpsCounter();
};