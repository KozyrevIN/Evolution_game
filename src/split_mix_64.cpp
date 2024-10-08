#define split_mix_64_header
#include "../include/split_mix_64.h"

SplitMix64::SplitMix64(uint64_t seed) : x(seed) {}

uint SplitMix64::next() {
    uint64_t z = (x += static_cast<uint64_t>(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * static_cast<uint64_t>(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * static_cast<uint64_t>(0x94D049BB133111EB);
    return (uint)(z ^ (z >> 31));
}
