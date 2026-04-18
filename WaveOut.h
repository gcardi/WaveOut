//---------------------------------------------------------------------------

#ifndef WaveOutH
#define WaveOutH

#include <windows.h>
#include <MMSystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

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
    static uint32_t GetDefaultSampleRate();
protected:
    WAVEFORMATEX waveFormat_;
    HWAVEOUT waveHandle_;
    std::vector<BufferCont> waveData_;
    std::vector<WAVEHDR> waveHeader_;
    size_t blockCnt_;
    std::atomic<size_t> freeBlockCnt_;

    std::atomic<bool> stopReq_ { false };
    std::atomic<bool> stopped_ { true };

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
    if ( !stopped_ ) {
        return;
    }

    stopReq_ = false;
    stopped_ = false;
    freeBlockCnt_ = 0;

    for ( size_t i = 0; i < blockCnt_; ++i ) {
        DoCallback( waveData_[i] );

        auto& Hdr = waveHeader_[i];
        ZeroMemory( &Hdr, sizeof( WAVEHDR ) );
        Hdr.lpData = reinterpret_cast<LPSTR>( waveData_[i].data() );
        Hdr.dwBufferLength = waveData_[i].size() * sizeof( SampleType );

        ::waveOutPrepareHeader( waveHandle_, &Hdr, sizeof( WAVEHDR ) );
        ::waveOutWrite( waveHandle_, &Hdr, sizeof( WAVEHDR ) );
    }
}
//---------------------------------------------------------------------------

template<typename T>
void WaveOut<T>::Stop()
{
    if ( stopped_ ) {
        return;
    }

    stopReq_ = true;
    freeBlockCnt_ = 0;
    exitEvt_->ResetEvent();
    ::waveOutReset( waveHandle_ );
    exitEvt_->WaitFor( INFINITE );

    for ( size_t i = 0; i < blockCnt_; ++i ) {
        if ( waveHeader_[i].dwFlags & WHDR_PREPARED ) {
            ::waveOutUnprepareHeader( waveHandle_, &waveHeader_[i], sizeof( WAVEHDR ) );
        }
    }

    stopped_ = true;
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
                if ( This->stopReq_ ) {
                    if ( ++This->freeBlockCnt_ >= This->blockCnt_ ) {
                        This->exitEvt_->SetEvent();
                    }
                }
                else {
                    auto idx = Hdr - This->waveHeader_.data();
                    This->DoCallback( This->waveData_[idx] );
                    ::waveOutWrite( hwo, Hdr, sizeof( WAVEHDR ) );
                }
            }
            break;
        case WOM_CLOSE:
            break;
    }
}

template<typename T>
uint32_t WaveOut<T>::GetDefaultSampleRate()
{
    uint32_t sampleRate = 44100;

    HRESULT hr = ::CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    bool comInitialized = SUCCEEDED( hr ) || hr == S_FALSE;

    if ( comInitialized || hr == RPC_E_CHANGED_MODE ) {
        IMMDeviceEnumerator* enumerator = nullptr;
        hr = ::CoCreateInstance(
            __uuidof( MMDeviceEnumerator ), nullptr, CLSCTX_ALL,
            __uuidof( IMMDeviceEnumerator ),
            reinterpret_cast<void**>( &enumerator )
        );
        if ( SUCCEEDED( hr ) ) {
            IMMDevice* device = nullptr;
            hr = enumerator->GetDefaultAudioEndpoint( eRender, eConsole, &device );
            if ( SUCCEEDED( hr ) ) {
                IAudioClient* audioClient = nullptr;
                hr = device->Activate(
                    __uuidof( IAudioClient ), CLSCTX_ALL, nullptr,
                    reinterpret_cast<void**>( &audioClient )
                );
                if ( SUCCEEDED( hr ) ) {
                    WAVEFORMATEX* mixFormat = nullptr;
                    hr = audioClient->GetMixFormat( &mixFormat );
                    if ( SUCCEEDED( hr ) ) {
                        sampleRate = mixFormat->nSamplesPerSec;
                        ::CoTaskMemFree( mixFormat );
                    }
                    audioClient->Release();
                }
                device->Release();
            }
            enumerator->Release();
        }
        if ( comInitialized ) {
            ::CoUninitialize();
        }
    }

    return sampleRate;
}

}
//---------------------------------------------------------------------------
#endif