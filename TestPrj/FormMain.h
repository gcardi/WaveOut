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
#include <Vcl.ExtCtrls.hpp>
#include "FrameLevelMeter.h"

#include <atomic>
#include <cstdint>
#include <memory>

#include "SineGen.h"
#include "FMGen.h"
#include "WhiteNoiseGen.h"
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
    TGroupBox *groupboxFMGen;
    TLabel *lblFMGenVol;
    TLabel *lblFMGenCarrierFreq;
    TLabel *lblFMGenModFreq;
    TLabel *lblFMGenModIndex;
    TTrackBar *trackbarFMGenVol;
    TTrackBar *trackbarFMGenCarrierFreq;
    TTrackBar *trackbarFMGenModFreq;
    TTrackBar *trackbarFMGenModIndex;
    TGroupBox *groupboxWhiteNoiseGen;
    TLabel *lblWhiteNoiseGenVol;
    TTrackBar *trackbarWhiteNoiseGenVol;
	TfrmeLevelMeter *frmeLevelMeter1;
    TTimer *tmrLevel;
    void __fastcall actStartExecute(TObject *Sender);
    void __fastcall EnabledIfStopped(TObject *Sender);
    void __fastcall actStopExecute(TObject *Sender);
    void __fastcall EnabledIfRunning(TObject *Sender);
    void __fastcall trackbarSineGenFreqChange(TObject *Sender);
    void __fastcall trackbarSineGenVolChange(TObject *Sender);
    void __fastcall trackbarFMGenVolChange(TObject *Sender);
    void __fastcall trackbarFMGenCarrierFreqChange(TObject *Sender);
    void __fastcall trackbarFMGenModFreqChange(TObject *Sender);
    void __fastcall trackbarFMGenModIndexChange(TObject *Sender);
    void __fastcall trackbarWhiteNoiseGenVolChange(TObject *Sender);
    void __fastcall tmrLevelTimer(TObject *Sender);
private:	// User declarations
    using WaveOutType = App::WaveOutCO<int16_t>;
    using WaveOutPtr = std::unique_ptr<WaveOutType>;

    WaveOutPtr wo_;
    static constexpr size_t bs = 1024;
    //static constexpr size_t sps = 22050;
    size_t const sps = WaveOutType::GetDefaultSampleRate();

    SineGen sineGen_{ this->SineGenFreq, sps, this->SineGenVol };
    FMGen fmGen_{ this->FMGenCarrierFreq, this->FMGenModFreq,
                  this->FMGenModIndex, sps, this->FMGenVol };
    WhiteNoiseGen whiteNoiseGen_{ 0x12345678u, this->WhiteNoiseGenVol };

    // Peak absolute output sample per audio buffer (linear, 0..1).
    // Audio thread: store relaxed. UI timer: load relaxed + write to meter.
    std::atomic<float> peakLevel_ {};

    float GetSineGenFreq() const;
    void SetSineGenFreq( float Val );

    float GetSineGenVol() const;
    void SetSineGenVol( float Val );

    float GetFMGenVol() const;
    void SetFMGenVol( float Val );

    float GetFMGenCarrierFreq() const;
    void SetFMGenCarrierFreq( float Val );

    float GetFMGenModFreq() const;
    void SetFMGenModFreq( float Val );

    float GetFMGenModIndex() const;
    void SetFMGenModIndex( float Val );

    float GetWhiteNoiseGenVol() const;
    void SetWhiteNoiseGenVol( float Val );

    __property float SineGenFreq = { read = GetSineGenFreq, write = SetSineGenFreq };
    __property float SineGenVol = { read = GetSineGenVol, write = SetSineGenVol };
    __property float FMGenVol = { read = GetFMGenVol, write = SetFMGenVol };
    __property float FMGenCarrierFreq = { read = GetFMGenCarrierFreq, write = SetFMGenCarrierFreq };
    __property float FMGenModFreq = { read = GetFMGenModFreq, write = SetFMGenModFreq };
    __property float FMGenModIndex = { read = GetFMGenModIndex, write = SetFMGenModIndex };
    __property float WhiteNoiseGenVol = { read = GetWhiteNoiseGenVol, write = SetWhiteNoiseGenVol };
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
