//---------------------------------------------------------------------------

#ifndef WhiteNoiseGenH
#define WhiteNoiseGenH

#include <atomic>
#include <bit>
#include <cstdint>

// Fast white-noise generator.
//
// A 32-bit xorshift PRNG drives the sequence: three shifts and XORs per
// sample, period 2^32 - 1, statistics flat enough for audio. The raw
// 32-bit output is turned into a float in [-1, 1) without any division
// or int->float conversion: bit-pattern (sign=0, exp=127, random 23-bit
// mantissa) yields a float in [1, 2), then 2*f - 3 maps it to [-1, 1).
class WhiteNoiseGen {
public:
    WhiteNoiseGen( std::uint32_t seed = 0x12345678u, float level = 1.0F )
      : level_{ level }
      , state_{ seed ? seed : 0x12345678u }   // xorshift needs non-zero state
    {
    }

    float operator()() const {
        std::uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x <<  5;
        state_ = x;

        float const f = std::bit_cast<float>(
                            0x3F800000u | ( x >> 9 ) );   // in [1, 2)
        auto const lvl = level_.load( std::memory_order_relaxed );
        return lvl * ( 2.0f * f - 3.0f );                 // in [-1, 1)
    }

    void SetLevel( float level ) {
        level_.store( level, std::memory_order_relaxed );
    }

    float GetLevel() const {
        return level_.load( std::memory_order_relaxed );
    }

private:
    std::atomic<float>    level_;
    mutable std::uint32_t state_;
};

//---------------------------------------------------------------------------
#endif
