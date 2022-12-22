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

#include <cstdint>
#include <memory>

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
    void __fastcall actStartExecute(TObject *Sender);
    void __fastcall EnabledIfStopped(TObject *Sender);
    void __fastcall actStopExecute(TObject *Sender);
    void __fastcall EnabledIfRunning(TObject *Sender);
private:	// User declarations
    using WaveOutType = App::WaveOutCO<int16_t>;
    using WaveOutPtr = std::unique_ptr<WaveOutType>;

    WaveOutPtr wo_;
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
