//---------------------------------------------------------------------------

#ifndef WaveOutH
#define WaveOutH

#include <windows.h>
#include <MMSystem.h>

#include <array>
#include <vector>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <Winapi.Messages.hpp>
#include <System.SyncObjs.hpp>
#include <System.SysUtils.hpp>

#include <memory>

namespace App {

template<typename T>
class WaveOut {
public:
    using SampleType = T;
    using BufferCont = std::vector<SampleType>;

    WaveOut( int Device, size_t Channels, size_t SampleCount, size_t BlockCount,
             uint32_t SamplesPerSec );
    virtual ~WaveOut();
    WaveOut( WaveOut const & ) = delete;
    WaveOut( WaveOut&& ) = delete;
    WaveOut& operator=( WaveOut const & ) = delete;
    WaveOut& operator=( WaveOut&& ) = delete;
    void Start();
    void Stop();
    [[nodiscard]] bool IsRunning() const { return !stopped_; }
protected:
    WAVEFORMATEX waveFormat_;
    HWAVEOUT waveHandle_;
    std::vector<BufferCont> waveData_;
    std::vector<WAVEHDR> waveHeader_;
    size_t blockCnt_;
    size_t freeBlockCnt_;

    std::atomic<bool> stopReq_ { false };
    std::atomic<bool> stopped_ { false };

    virtual void DoCallback( BufferCont& WaveData ) const = 0;

    static void CALLBACK WaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
                                      DWORD_PTR dwParam1, DWORD_PTR dwParam2 );
private:

    std::unique_ptr<TEvent> exitEvt_ {
        new TEvent( 0, false, false, EmptyStr, false )
    };
};

template<typename T = int16_t>
class WaveOutCO : public WaveOut<T> {
public:
    using CallableObjType = std::function<void(typename WaveOut<T>::BufferCont&)>;

    WaveOutCO( int Device, size_t Channels, size_t SampleCount, size_t BlockCount,
               uint32_t SamplesPerSec, CallableObjType Callback )
      : WaveOut<T>( Device, Channels, SampleCount, BlockCount, SamplesPerSec )
      , callback_{ Callback }
    {
    }
protected:
    virtual void DoCallback( typename WaveOut<T>::BufferCont& WaveData ) const override {
        callback_( WaveData );
    }
private:
    CallableObjType callback_;
};

template<typename T>
WaveOut<T>::WaveOut( int Device, size_t Channels, size_t SampleCount,
                     size_t BlockCount, uint32_t SamplesPerSec )
    : blockCnt_{ BlockCount }, freeBlockCnt_{ BlockCount }
{
    waveData_.resize( BlockCount );
    for ( auto& WD : waveData_ ) {
        WD.resize( SampleCount );
    }

    waveHeader_.resize( BlockCount );

    waveFormat_ = { 0 };
    waveFormat_.wFormatTag      = WAVE_FORMAT_PCM;
    waveFormat_.nChannels       = Channels;
    waveFormat_.nSamplesPerSec  = SamplesPerSec;
    waveFormat_.wBitsPerSample  = sizeof( SampleType ) * 8;
    waveFormat_.nAvgBytesPerSec =
        SamplesPerSec *
        waveFormat_.wBitsPerSample / 8 *
        waveFormat_.nChannels;
    waveFormat_.nBlockAlign     =
        waveFormat_.nChannels * waveFormat_.wBitsPerSample / 8;
    waveFormat_.cbSize          = 0;

    auto Res = ::waveOutOpen(
        &waveHandle_,
        Device,
        &waveFormat_,
        reinterpret_cast<DWORD_PTR>( &WaveOutProc ),
        reinterpret_cast<DWORD_PTR>( this ),
        CALLBACK_FUNCTION
    );
    if ( Res ) {
        RaiseLastOSError();
    }
}

template<typename T>
WaveOut<T>::~WaveOut()
{
    Stop();
    ::waveOutClose( waveHandle_ );
}
//---------------------------------------------------------------------------

// https://gist.github.com/seungin/4779216eada24a5077ca1c5e857239ce

template<typename T>
void WaveOut<T>::Start()
{
}
//---------------------------------------------------------------------------

template<typename T>
void WaveOut<T>::Stop()
{
}
//---------------------------------------------------------------------------

template<typename T>
void WaveOut<T>::WaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
                              DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
    auto This = reinterpret_cast<WaveOut*>( dwInstance );

    switch ( uMsg ) {
        case WOM_OPEN:
            break;
        case WOM_DONE:
            if ( auto const Hdr = reinterpret_cast<WAVEHDR *>( dwParam1 ) ) {
            }
            break;
        case WOM_CLOSE:
            break;

    }
}

}
//---------------------------------------------------------------------------
#endif