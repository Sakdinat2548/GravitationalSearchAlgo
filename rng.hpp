// Simple xoshiro256++ implementation (public domain style)
#pragma once

#include <cstdint>
#include <random>

// SplitMix64 for seeding
static uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

class Xoshiro256PlusPlus {
  public:
    using result_type = uint64_t;

    Xoshiro256PlusPlus() { seed(std::random_device{}()); }
    explicit Xoshiro256PlusPlus(uint64_t seed_val) { seed(seed_val); }

    void seed(uint64_t seed_val) {
        uint64_t z = seed_val;
        s[0] = splitmix64(z);
        s[1] = splitmix64(z);
        s[2] = splitmix64(z);
        s[3] = splitmix64(z);
    }

    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() { return UINT64_C(0xffffffffffffffff); }

    inline uint64_t operator()() {
        const uint64_t result = rol(s[0] + s[3], 23) + s[0];

        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rol(s[3], 45);

        return result;
    }

  private:
    uint64_t s[4];

    static inline uint64_t rol(const uint64_t x, int k) noexcept {
        return (x << k) | (x >> (64 - k));
    }
};
