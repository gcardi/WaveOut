//---------------------------------------------------------------------------
// Utils.h
// Minimal subset of the PitchDetector Utils header — just the dB helpers
// needed by FrameLevelMeter. Kept in the PitchDet namespace so the ported
// frame compiles unmodified.
//---------------------------------------------------------------------------

#ifndef UtilsH
#define UtilsH

#include <algorithm>
#include <cmath>

namespace PitchDet {

[[nodiscard]] inline float dBToValue( float Value )
{
    return std::pow( 10.0F, Value / 20.0F );
}

[[nodiscard]] inline float ValueTodB( float Value )
{
    static constexpr float Neg300dBValue = 1e-15F;
    return 20.0F * std::log10( std::max( Value, Neg300dBValue ) );
}

} // namespace PitchDet

//---------------------------------------------------------------------------
#endif
