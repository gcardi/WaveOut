//---------------------------------------------------------------------------

#ifndef FrameLevelMeterH
#define FrameLevelMeterH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>

#include <optional>
#include <memory>
#include <cmath>

#include <GdiPlus.h>

#include "Utils.h"

//---------------------------------------------------------------------------

class TfrmeLevelMeter : public TFrame
{
__published:	// IDE-managed Components
    TPaintBox *PaintBox1;
    TTimer *tmrPeak;
    void __fastcall PaintBox1Paint(TObject *Sender);
    void __fastcall FrameResize(TObject *Sender);
    void __fastcall tmrPeakTimer(TObject *Sender);
private:	// User declarations

    static constexpr float DefHighLim_dB = 0.0F;
    static constexpr float DefLowLim_dB = -96.0F;

    float value_ {};
    std::optional<float> peakValue_ {};
    float hiLim_dB_ { DefHighLim_dB };
    float loLim_dB_ { DefLowLim_dB };
    float hiLim_ { PitchDet::dBToValue( DefHighLim_dB ) };
    float loLim_ { PitchDet::dBToValue( DefLowLim_dB ) };
    TRect offRegion_;
    TRect onRegion_;
    std::unique_ptr<TBitmap> bmpBack_;
    std::unique_ptr<Gdiplus::Bitmap> bmpScale_;
    float scaleLimitAt_ { 1.0F - PitchDet::dBToValue( -0.0F ) };
    float maxLevelAt_ { 1.0F - PitchDet::dBToValue( -3.0F ) };

    void SetValue( float Value );
    void SetHiLim_dB( float Value );
    void SetLoLim_dB( float Value );
    void UpdateRegions();
    [[nodiscard]] int ComputeYPos( float dB ) const;
    void CreateGradientFillBitmap();
    void DestroyBitmaps();
    void CreateBitmaps1();
#if !defined( DONT_USE_SCALE_BITMAP )
    void CreateScaleBitmap();
#endif
public:		// User declarations
    __fastcall TfrmeLevelMeter(TComponent* Owner);
    void Reset();
    __property float Value = { read = value_, write = SetValue };
    __property float HiLim_dB = { read = hiLim_dB_, write = SetHiLim_dB };
    __property float LoLim_dB = { read = loLim_dB_, write = SetLoLim_dB };
};
//---------------------------------------------------------------------------
//extern PACKAGE TfrmeLevelMeter *frmeLevelMeter;
//---------------------------------------------------------------------------
#endif
