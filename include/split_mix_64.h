class SplitMix64 {
private:
    uint64_t x;

public:
    SplitMix64(uint64_t seed);

    uint64_t Next();
};