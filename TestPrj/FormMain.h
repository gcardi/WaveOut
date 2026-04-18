//---------------------------------------------------------------------------

#ifndef FormMainH
#define FormMainH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ActnMan.hpp>
#include <Vcl.PlatformDefaultStyleActnCtrls.hpp>
#include <Vcl.ComCtrls.hpp>

#include <cstdint>
#include <memory>

#include "SineGen.h"
#include "FMGen.h"
#include "WaveOut.h"

//---------------------------------------------------------------------------

class TfrmMain : public TForm
{
__published:	// IDE-managed Components
    TActionManager *ActionManager1;
    TAction *actStart;
    TAction *actStop;
    TButton *Button1;
    TButton *Button2;
    TGroupBox *groupboxSineGen;
    TTrackBar *trackbarSineGenVol;
    TLabel *lblSineGenVol;
    TLabel *lblSineGenFreq;
    TTrackBar *trackbarSineGenFreq;
    void __fastcall actStartExecute(TObject *Sender);
    void __fastcall EnabledIfStopped(TObject *Sender);
    void __fastcall actStopExecute(TObject *Sender);
    void __fastcall EnabledIfRunning(TObject *Sender);
    void __fastcall trackbarSineGenFreqChange(TObject *Sender);
    void __fastcall trackbarSineGenVolChange(TObject *Sender);
private:	// User declarations
    using WaveOutType = App::WaveOutCO<int16_t>;
    using WaveOutPtr = std::unique_ptr<WaveOutType>;

    WaveOutPtr wo_;
    static constexpr size_t bs = 1024;
    //static constexpr size_t sps = 22050;
    size_t const sps = WaveOutType::GetDefaultSampleRate();

    SineGen g_{ this->SineGenFreq, sps };
    //FMGen g_{ 440.0F, 20.0F, 2.0F, sps };

    float GetSineGenFreq() const;
    void SetSineGenFreq( float Val );

    float GetSineGenVol() const;
    void SetSineGenVol( float Val );

    __property float SineGenFreq = { read = GetSineGenFreq, write = SetSineGenFreq };
    __property float SineGenVol = { read = GetSineGenVol, write = SetSineGenVol };
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
