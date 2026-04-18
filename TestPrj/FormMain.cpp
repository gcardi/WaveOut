//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <cstdint>
#include <limits>
#include <algorithm>

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

void __fastcall TfrmMain::actStartExecute(TObject *Sender)
{
    //
    Caption = Format( _D( "Sample Rate = %d" ), sps );

    wo_ =
        make_unique<WaveOutType>(
            WAVE_MAPPER,// int Device
            1,          // size_t Channels
            bs,         // size_t SampleCount
            8,          // size_t BlockCount
            sps,        // uint32_t SamplesPerSec
            // CallableObjType Callback
            [this]( auto& Buffer ) {
                //Buffer[0] = 10;
                for ( auto& Sample : Buffer ) {
                    using SampleType = WaveOutType::SampleType;
                    Sample =
                        static_cast<SampleType>(
                            static_cast<float>( std::numeric_limits<SampleType>::max() ) *
                            g_()
                        );
                }
            }
        );
    wo_->Start();

}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::EnabledIfStopped(TObject *Sender)
{
    auto& Act = static_cast<TAction&>( *Sender );
    Act.Enabled = !wo_;
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
    auto& Act = static_cast<TAction&>( *Sender );
    Act.Enabled = !!wo_;
}
//---------------------------------------------------------------------------

float TfrmMain::GetSineGenFreq() const
{
    return trackbarSineGenFreq->Position;
}
//---------------------------------------------------------------------------

void TfrmMain::SetSineGenFreq( float Val )
{
    Val =
        std::clamp(
            Val,
            static_cast<float>( trackbarSineGenFreq->Min ),
            static_cast<float>( trackbarSineGenFreq->Max )
        );
    //trackbarSineGenFreq->Position = Val;
    g_.SetFreq( Val );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarSineGenFreqChange(TObject *Sender)
{
    SineGenFreq = trackbarSineGenFreq->Position;
}
//---------------------------------------------------------------------------

float TfrmMain::GetSineGenVol() const
{
    return trackbarSineGenVol->Position / 100.0F;
}
//---------------------------------------------------------------------------

void TfrmMain::SetSineGenVol( float Val )
{
    Val = std::clamp( Val, 0.0F, 1.0F );
    g_.SetLevel( Val );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarSineGenVolChange(TObject *Sender)
{
    SineGenVol = trackbarSineGenVol->Position / 100.0F;
}
//---------------------------------------------------------------------------

