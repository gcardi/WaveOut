# WaveOut

A lightweight, header-only C++ library for real-time audio output on Windows, built on top of the Win32 Multimedia (`waveOut`) API. Designed for use with Embarcadero C++ Builder (bcc64/bcc64x) but portable to any Windows C++ toolchain with minor adjustments.

## Design philosophy

The library wraps the low-level `waveOut` callback mechanism into a modern C++ template class that follows a few key principles:

- **Header-only** — the entire library lives in a single header file (`WaveOut.h`), making integration trivial.
- **Template-based** — `WaveOut<T>` is parameterized on the sample type (e.g. `int16_t`, `float`), so the same code works for different PCM formats without casts or conversions at the call site.
- **Double-buffered by design** — the constructor accepts a `BlockCount` parameter that controls how many audio buffers are kept in flight. The OS plays one buffer while the application fills the next, eliminating gaps.
- **Callback-driven** — a pure virtual method `DoCallback()` is invoked from the multimedia thread whenever a buffer needs to be refilled. The derived class `WaveOutCO` wraps this into a `std::function`, so you can supply a lambda directly.
- **Thread-aware** — the callback runs on a separate multimedia thread managed by Windows. The library uses `std::atomic` for synchronization and a `TEvent` for clean shutdown, avoiding mutexes in the audio path.

## Class hierarchy

```
WaveOut<T>          Base template: opens the device, manages buffers, Start/Stop
  │
  └─ WaveOutCO<T>  Convenience subclass: takes a std::function callback
```

### `WaveOut<T>`

| Parameter | Description |
|---|---|
| `Device` | Audio device index (`WAVE_MAPPER` for default) |
| `Channels` | Number of channels (1 = mono, 2 = stereo) |
| `SampleCount` | Number of samples per buffer |
| `BlockCount` | Number of buffers to keep in flight |
| `SamplesPerSec` | Sample rate in Hz |

Key members:

- `Start()` / `Stop()` — begin and end playback. `Stop()` waits for all in-flight buffers to drain before returning.
- `GetDefaultSampleRate()` — queries the system's default audio endpoint via WASAPI to discover the native sample rate, falling back to 44100 Hz.
- `DoCallback(BufferCont&)` — pure virtual; override this to fill the buffer with audio data.

### `WaveOutCO<T>`

A thin wrapper that accepts a `std::function<void(BufferCont&)>` in its constructor and forwards `DoCallback` to it. This lets you write the audio generation as an inline lambda.

## Demo application (`TestPrj/`)

The demo is a VCL (Visual Component Library) application built with C++ Builder. It demonstrates real-time audio synthesis by mixing three generators, displays the output level on a graphical meter, and allows live parameter control from the GUI.

### Generators

- **`SineGen`** — a sine wave oscillator driven by a phase accumulator. Samples are read from a compile-time sine LUT (see [SineLUT](#sinelut)) with linear interpolation between entries. Parameters: frequency (Hz) and level (0.0–1.0).
- **`FMGen`** — an FM (Frequency Modulation) synthesis generator that also reads from the same LUT. The output follows the classic FM equation: $y = \text{level} \cdot \sin(\phi_c + I \cdot \sin(\phi_m))$, where $\phi_c$ and $\phi_m$ are the accumulated carrier and modulator phases, and $I$ is the modulation index. Parameters: carrier frequency, modulator frequency, modulation index, and level.
- **`WhiteNoiseGen`** — a fast white-noise generator. A 32-bit xorshift PRNG produces the sequence (three shift/XOR pairs per sample, period $2^{32}-1$). The 32-bit word is converted to a float in $[-1, 1)$ without any division or int-to-float conversion: `std::bit_cast` builds an IEEE-754 float in $[1, 2)$ from (sign=0, exponent=127, random 23-bit mantissa), then `2f - 3` maps it to $[-1, 1)$. Parameter: level (0.0–1.0).

All three generators use `std::atomic<float>` with relaxed memory ordering for their tuneable parameters, so they can be adjusted from the GUI (VCL main thread) while the audio callback runs on the multimedia thread — no mutex, no blocking, no priority inversion.

### SineLUT

`SineTable.h` defines a 4096-entry sine lookup table in the `SineLUT` namespace. The table is populated at compile time through a `constexpr` factory using a 6-term Taylor-series approximation with range reduction — this is needed because `std::sin` isn't `constexpr` before C++26, and the Clang shipped with C++Builder 13 doesn't accept `__builtin_sinf` in constant expressions. The result lives in read-only program memory, making the same code suitable for MCU targets (the companion STM32Gen project shares the technique). At runtime, `SineGen` and `FMGen` index the table with a phase accumulator and linearly interpolate between adjacent entries.

### Level meter

`FrameLevelMeter` is a reusable VCL `TFrame` that renders a vertical peak-level bar driven by GDI+. The frame:

- Converts the incoming linear value to dB (`Utils.h::PitchDet::dBToValue` / `ValueTodB`) and maps it onto a configurable `HiLim_dB` / `LoLim_dB` range (defaults: 0 dB / −96 dB).
- Caches a gradient-filled background bitmap and an optional scale bitmap, repainting only the "on" and "off" regions on each update.
- Uses an internal `TTimer` to decay a peak-hold marker.

The audio callback tracks the per-buffer peak in an `std::atomic<float>`; a UI timer in the main form loads that value and writes it to the meter's `Value` property. `GdiPlusUtils` (see below) provides the GDI+ initialisation scope and conversion helpers.

### GdiPlusUtils

`GdiPlusUtils.{h,cpp}` is a small helper layer around the Windows GDI+ C++ API:

- `GdiPlusSessionManager` — RAII wrapper around `GdiplusStartup` / `GdiplusShutdown`. Instantiate one at application scope.
- `EGdiplusException` / `GdiplusCheck` — exception type and helper that turns a non-OK `Gdiplus::Status` into a VCL `Exception` with a human-readable message.
- Conversion helpers: `TColorToGdiplusColor` (with and without alpha), `AlignmentToGdiplusStringAlignment`, `VerticalAlignmentToGdiplusStringAlignment`, `VCLRectToGdiPlusRectF`.
- `LoadImage`, `DrawImage` (with optional stretching / aspect preservation), and `GetEncoderClsid` for codec lookup.

### GUI layout

The form contains three generator group boxes plus the level meter:

| Sine Gen | FM Gen | White Noise Gen | Level Meter |
|---|---|---|---|
| Volume (0–100%) | Volume (0–100%) | Volume (0–100%) | Peak meter (dB) |
| Frequency (20–10000 Hz) | Carrier Frequency (20–10000 Hz) | | |
| | Modulator Frequency (1–5000 Hz) | | |
| | Modulation Index (0.0–10.0) | | |

The three generators are mixed with equal weight in the audio callback — `(1/3) * (sineGen + fmGen + whiteNoiseGen)` — and each generator's individual volume trackbar controls its relative contribution. The callback also tracks the per-buffer peak absolute sample in `peakLevel_` (relaxed atomic); a UI timer (`tmrLevel`) reads that value and writes it to the level meter frame.

**Start** and **Stop** buttons control playback. The window title displays the detected native sample rate.

### Building

Open `TestPrj/Test.cbproj` in RAD Studio / C++ Builder and build for the Win64 (Modern) platform.

## License

MIT — see [LICENSE](LICENSE).
