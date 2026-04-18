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

The demo is a VCL (Visual Component Library) application built with C++ Builder. It demonstrates real-time audio synthesis by mixing two generators and allowing live parameter control from the GUI.

### Generators

- **`SineGen`** — a simple sine wave oscillator using a phase accumulator. Parameters: frequency (Hz) and level (0.0–1.0).
- **`FMGen`** — an FM (Frequency Modulation) synthesis generator. The output follows the classic FM equation: $y = \text{level} \cdot \sin(\phi_c + I \cdot \sin(\phi_m))$, where $\phi_c$ and $\phi_m$ are the accumulated carrier and modulator phases, and $I$ is the modulation index. Parameters: carrier frequency, modulator frequency, modulation index, and level.

Both generators use `std::atomic<float>` with relaxed memory ordering for all tuneable parameters, making them safe to adjust from the GUI (VCL main thread) while the audio callback runs on the multimedia thread — no mutex, no blocking, no priority inversion.

### GUI layout

The form contains two group boxes with trackbar controls:

| Sine Gen | FM Gen |
|---|---|
| Volume (0–100%) | Volume (0–100%) |
| Frequency (20–10000 Hz) | Carrier Frequency (20–10000 Hz) |
| | Modulator Frequency (1–5000 Hz) |
| | Modulation Index (0.0–10.0) |

The two generators are mixed with equal weight (`0.5 * (sineGen + fmGen)`) in the audio callback, and each generator's individual volume trackbar controls its relative contribution.

**Start** and **Stop** buttons control playback. The window title displays the detected native sample rate.

### Building

Open `TestPrj/Test.cbproj` in RAD Studio / C++ Builder and build for the Win64 (Modern) platform.

## License

MIT — see [LICENSE](LICENSE).
