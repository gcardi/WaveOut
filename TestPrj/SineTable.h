//---------------------------------------------------------------------------
// SineTable.h
// Compile-time generated sine lookup table (LUT) for fast waveform synthesis.
//
// The table is populated at compile time through a constexpr factory, so the
// samples live in read-only program memory rather than being computed during
// static initialization. Same technique used in the companion STM32Gen
// project (Core/Src/WaveGenTask.cpp).
//
// Note: std::sin is not guaranteed constexpr before C++26, and the Clang
// built-in __builtin_sinf is not accepted in constant expressions by the
// version of Clang shipped with Embarcadero C++Builder 13. A small Taylor
// series approximation is therefore used to populate the LUT — precision
// well within float tolerance for the 4096-entry table, and the result is
// smoothed further by linear interpolation at lookup time.
//---------------------------------------------------------------------------

#ifndef SineTableH
#define SineTableH

#include <array>
#include <cstddef>

namespace SineLUT {

constexpr std::size_t TableSize = 4096;

static_assert( ( TableSize & ( TableSize - 1 ) ) == 0,
               "TableSize must be a power of two (bit-mask index wrap)." );

constexpr float Pi     = M_PI; //3.14159265358979323846f;
constexpr float TwoPi  = 2.0f * Pi;
constexpr float HalfPi = 0.5f * Pi;

using TableStorage = std::array<float, TableSize>;

// Constexpr sine via range reduction + Taylor series (6 terms, up to x^11).
// Accurate to ~1e-7 over the full circle, which is below float precision.
constexpr float ConstSin( float x ) {
    // Reduce to [-pi, pi].
    while ( x >  Pi ) x -= TwoPi;
    while ( x < -Pi ) x += TwoPi;
    // Fold to [-pi/2, pi/2] using sin(pi - x) = sin(x).
    if ( x >  HalfPi ) x =  Pi - x;
    if ( x < -HalfPi ) x = -Pi - x;

    float const x2 = x * x;
    float term = x;
    float result = term;
    term *= -x2 / (  2.0f *  3.0f );  result += term;
    term *= -x2 / (  4.0f *  5.0f );  result += term;
    term *= -x2 / (  6.0f *  7.0f );  result += term;
    term *= -x2 / (  8.0f *  9.0f );  result += term;
    term *= -x2 / ( 10.0f * 11.0f );  result += term;
    return result;
}

constexpr float GenerateSample( std::size_t i ) {
    return ConstSin( TwoPi * static_cast<float>( i ) /
                             static_cast<float>( TableSize ) );
}

constexpr TableStorage CreateTable() {
    TableStorage t = {};
    for ( std::size_t i = 0; i < TableSize; ++i ) {
        t[i] = GenerateSample( i );
    }
    return t;
}

constexpr auto Table = CreateTable();

// Pre-computed scale and bias for the runtime lookup. The bias is added in
// index space so the float-to-int truncation is equivalent to a floor: it
// covers any realistic negative phase (~ -100 rad, i.e. modulation depth well
// past what an audio FM patch produces) while keeping the scaled index small
// enough to preserve float32 sub-sample precision (ULP ~ 0.01).
constexpr std::size_t IndexMask    = TableSize - 1;
constexpr float       PhaseScale   = static_cast<float>( TableSize ) / TwoPi;
constexpr std::size_t BiasCycles   = 16;
constexpr std::size_t IndexBias    = BiasCycles * TableSize;
constexpr float       IndexBiasF   = static_cast<float>( IndexBias );

// Fast lookup with linear interpolation. Runtime cost: 3 muls, 2 adds,
// 1 sub, 1 float->int cast, 1 int->float cast, 2 bit-ands, 2 loads.
// No fmod, no division. Phase must satisfy phase > -BiasCycles*2pi.
inline float Get( float phase ) {
    float       const idx  = phase * PhaseScale + IndexBiasF;
    std::size_t const iInt = static_cast<std::size_t>( idx );
    std::size_t const i0   = iInt & IndexMask;
    std::size_t const i1   = ( i0 + 1 ) & IndexMask;
    float       const frac = idx - static_cast<float>( iInt );
    return Table[i0] * ( 1.0f - frac ) + Table[i1] * frac;
}

} // namespace SineLUT

//---------------------------------------------------------------------------
#endif
