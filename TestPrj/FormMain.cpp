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
                for ( auto& Sample : Buffer ) {
                    using SampleType = WaveOutType::SampleType;
                    auto const mix = 0.5F * ( sineGen_() + fmGen_() );
                    Sample =
                        static_cast<SampleType>(
                            static_cast<float>( std::numeric_limits<SampleType>::max() ) *
                            mix
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
    sineGen_.SetFreq( Val );
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
    sineGen_.SetLevel( Val );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarSineGenVolChange(TObject *Sender)
{
    SineGenVol = trackbarSineGenVol->Position / 100.0F;
}
//---------------------------------------------------------------------------

float TfrmMain::GetFMGenVol() const
{
    return trackbarFMGenVol->Position / 100.0F;
}
//---------------------------------------------------------------------------

void TfrmMain::SetFMGenVol( float Val )
{
    Val = std::clamp( Val, 0.0F, 1.0F );
    fmGen_.SetLevel( Val );
}
//---------------------------------------------------------------------------

float TfrmMain::GetFMGenCarrierFreq() const
{
    return trackbarFMGenCarrierFreq->Position;
}
//---------------------------------------------------------------------------

void TfrmMain::SetFMGenCarrierFreq( float Val )
{
    Val =
        std::clamp(
            Val,
            static_cast<float>( trackbarFMGenCarrierFreq->Min ),
            static_cast<float>( trackbarFMGenCarrierFreq->Max )
        );
    fmGen_.SetCarrierFreq( Val );
}
//---------------------------------------------------------------------------

float TfrmMain::GetFMGenModFreq() const
{
    return trackbarFMGenModFreq->Position;
}
//---------------------------------------------------------------------------

void TfrmMain::SetFMGenModFreq( float Val )
{
    Val =
        std::clamp(
            Val,
            static_cast<float>( trackbarFMGenModFreq->Min ),
            static_cast<float>( trackbarFMGenModFreq->Max )
        );
    fmGen_.SetModulatorFreq( Val );
}
//---------------------------------------------------------------------------

float TfrmMain::GetFMGenModIndex() const
{
    return trackbarFMGenModIndex->Position / 10.0F;
}
//---------------------------------------------------------------------------

void TfrmMain::SetFMGenModIndex( float Val )
{
    Val = std::clamp( Val, 0.0F, 10.0F );
    fmGen_.SetModIndex( Val );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarFMGenVolChange(TObject *Sender)
{
    FMGenVol = trackbarFMGenVol->Position / 100.0F;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarFMGenCarrierFreqChange(TObject *Sender)
{
    FMGenCarrierFreq = trackbarFMGenCarrierFreq->Position;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarFMGenModFreqChange(TObject *Sender)
{
    FMGenModFreq = trackbarFMGenModFreq->Position;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trackbarFMGenModIndexChange(TObject *Sender)
{
    FMGenModIndex = trackbarFMGenModIndex->Position / 10.0F;
}
//---------------------------------------------------------------------------

