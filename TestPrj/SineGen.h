//---------------------------------------------------------------------------

#ifndef SineGenH
#define SineGenH

#include <cmath>
#include <atomic>

#include "SineTable.h"

class SineGen {
public:
    SineGen( float f, size_t sps, float level = 1.0F )
      : sps_{ sps }
      , level_{ level }
      , dPhase_{ static_cast<float>( 2.0 * M_PI ) * f /
                 static_cast<float>( sps ) }
    {
    }

    float operator()() const {
        auto const dP = dPhase_.load( std::memory_order_relaxed );
        auto const lvl = level_.load( std::memory_order_relaxed );
        auto const out = lvl * SineLUT::Get( phase_ );
        phase_ = std::fmod( phase_ + dP,
                            static_cast<float>( 2.0 * M_PI ) );
        return out;
    }

    void SetFreq( float f ) {
        dPhase_.store(
            static_cast<float>( 2.0 * M_PI ) * f /
            static_cast<float>( sps_ ),
            std::memory_order_relaxed
        );
    }

    float GetFreq() const {
        auto const dP = dPhase_.load( std::memory_order_relaxed );
        return dP * static_cast<float>( sps_ ) /
               static_cast<float>( 2.0 * M_PI );
    }

    void SetLevel( float level ) {
        level_.store( level, std::memory_order_relaxed );
    }

    float GetLevel() const {
        return level_.load( std::memory_order_relaxed );
    }

private:
    size_t sps_;
    std::atomic<float> level_;
    std::atomic<float> dPhase_;
    mutable float phase_ {};
};


//---------------------------------------------------------------------------
#endif
