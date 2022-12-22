//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

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
    wo_ =
        make_unique<WaveOutType>(
            WAVE_MAPPER,     // int Device
            1,      // size_t Channels
            1024,   // size_t SampleCount,
            22050,  // uint32_t SamplesPerSec,
            // CallableObjType Callback
            []( auto& Buffer ) {
                Buffer[0] = 10;
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

