//---------------------------------------------------------------------------

#ifndef FMGenH
#define FMGenH

#include <cmath>
#include <atomic>

class FMGen {
public:
    FMGen( float carrierFreq, float modulatorFreq, float modIndex,
           size_t sps, float level = 1.0F )
      : sps_{ sps }
      , level_{ level }
      , mi_{ modIndex }
      , dPhaseC_{ static_cast<float>( 2.0 * M_PI ) * carrierFreq /
                  static_cast<float>( sps ) }
      , dPhaseM_{ static_cast<float>( 2.0 * M_PI ) * modulatorFreq /
                  static_cast<float>( sps ) }
    {
    }

    float operator()() const {
        auto const mi = mi_.load( std::memory_order_relaxed );
        auto const dC = dPhaseC_.load( std::memory_order_relaxed );
        auto const dM = dPhaseM_.load( std::memory_order_relaxed );
        auto const lvl = level_.load( std::memory_order_relaxed );
        auto const mod = mi * std::sin( phaseM_ );
        auto const out = lvl * std::sin( phaseC_ + mod );
        phaseC_ = std::fmod( phaseC_ + dC,
                             static_cast<float>( 2.0 * M_PI ) );
        phaseM_ = std::fmod( phaseM_ + dM,
                             static_cast<float>( 2.0 * M_PI ) );
        return out;
    }

    void SetCarrierFreq( float f ) {
        dPhaseC_.store(
            static_cast<float>( 2.0 * M_PI ) * f /
            static_cast<float>( sps_ ),
            std::memory_order_relaxed
        );
    }

    void SetModulatorFreq( float f ) {
        dPhaseM_.store(
            static_cast<float>( 2.0 * M_PI ) * f /
            static_cast<float>( sps_ ),
            std::memory_order_relaxed
        );
    }

    void SetModIndex( float mi ) {
        mi_.store( mi, std::memory_order_relaxed );
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
    std::atomic<float> mi_;
    std::atomic<float> dPhaseC_;
    std::atomic<float> dPhaseM_;
    mutable float phaseC_ {};
    mutable float phaseM_ {};
};

//---------------------------------------------------------------------------
#endif
