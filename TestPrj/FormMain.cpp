//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <cmath>
#include <cstdint>
#include <limits>

#include "FormMain.h"

using std::make_unique;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------

__fastcall TfrmMain::TfrmMain(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

class Gen {
public:
    Gen( float f, size_t sps )
      : f_{ f }, d_{ 1.0F / static_cast<float>( sps ) } {}
    float operator()() const {
        auto out = sin( 2.0 * M_PI * f_ * t_ );
        t_ = fmod( t_ + d_, static_cast<float>( 2.0 * M_PI ) );
        return out;
    }
private:
    float f_;
    mutable float t_ {};
    float d_;
};

static constexpr size_t bs = 1024;
static constexpr size_t sps = 22050;

Gen g( 440.0F, sps );
float st {};

void __fastcall TfrmMain::actStartExecute(TObject *Sender)
{
    //


    wo_ =
        make_unique<WaveOutType>(
            WAVE_MAPPER,// int Device
            1,          // size_t Channels
            bs,         // size_t SampleCount
            8,          // size_t BlockCount
            sps,        // uint32_t SamplesPerSec
            // CallableObjType Callback
            []( auto& Buffer ) {
                //Buffer[0] = 10;
                for ( auto& Sample : Buffer ) {
                    using SampleType = WaveOutType::SampleType;
                    Sample =
                        static_cast<SampleType>(
                            static_cast<float>( std::numeric_limits<SampleType>::max() ) *
                            g()
                        );
                }
            }
        );

}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::EnabledIfStopped(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actStopExecute(TObject *Sender)
{
//
    wo_.reset();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::EnabledIfRunning(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

