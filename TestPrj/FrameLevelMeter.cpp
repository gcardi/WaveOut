//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Vcl.Direct2D.hpp>
#include <Winapi.D2d1.hpp>

#include <cmath>
#include <array>
#include <algorithm>

#include "GdiPlusUtils.h"

#include "FrameLevelMeter.h"

using std::max;
using std::array;
using std::make_unique;

using Winapi::D2d1::_di_ID2D1LinearGradientBrush;
using Winapi::D2d1::TD2D1LinearGradientBrushProperties;

using PitchDet::dBToValue;
using PitchDet::ValueTodB;

#if !defined( DONT_USE_SCALE_BITMAP )
using Gdiplus::Pen;
using Gdiplus::SolidBrush;
using Gdiplus::Color;
using GdiplusUtils::GdiplusCheck;
using Gdiplus::SmoothingModeAntiAlias;
using Gdiplus::StringFormat;
using Gdiplus::Bitmap;
#endif

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TfrmeLevelMeter *frmeLevelMeter;
#pragma comment( lib, "gdiplus" )
//---------------------------------------------------------------------------

__fastcall TfrmeLevelMeter::TfrmeLevelMeter(TComponent* Owner)
    : TFrame(Owner)
{
    ControlStyle = ControlStyle << csOpaque;
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::SetHiLim_dB( float Value )
{
    if ( hiLim_dB_ != Value ) {
        hiLim_dB_ = Value;
        hiLim_ = dBToValue( Value );
        DestroyBitmaps();
        UpdateRegions();
        Invalidate();
    }
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::SetLoLim_dB( float Value )
{
    if ( loLim_dB_ != Value ) {
        loLim_dB_ = Value;
        loLim_ = dBToValue( Value );
        DestroyBitmaps();
        UpdateRegions();
        Invalidate();
    }
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::SetValue( float Value )
{
    if ( value_ != Value ) {
        value_ = Value;
        if ( peakValue_.has_value() ) {
            if ( value_ > *peakValue_ ) {
                peakValue_ = value_;
                if ( value_ > loLim_ ) {
                    tmrPeak->Enabled = false;
                    tmrPeak->Interval = 750;
                    tmrPeak->Enabled = true;
                }
            }
        }
        else {
            peakValue_ = value_;
            if ( value_ > loLim_ ) {
                tmrPeak->Interval = 500;
                tmrPeak->Enabled = true;
            }
        }
        UpdateRegions();
        Repaint();
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmeLevelMeter::tmrPeakTimer(TObject *Sender)
{
    tmrPeak->Enabled = false;
    auto dB = ValueTodB( *peakValue_ ) - 2.0F;
    if ( dB < loLim_dB_ ) {
        peakValue_.reset();
    }
    else {
        peakValue_ = dBToValue( dB );
        tmrPeak->Interval = 20;
        tmrPeak->Enabled = true;
    }
    Repaint();
}
//---------------------------------------------------------------------------

int TfrmeLevelMeter::ComputeYPos( float dB ) const
{
    auto Rect = PaintBox1->ClientRect;
    return
        ( Rect.Bottom * hiLim_dB_ -
          loLim_dB_ * Rect.Top -
          Rect.Bottom * dB +
          Rect.Top * dB
        ) /
        ( hiLim_dB_ - loLim_dB_ );
}
//---------------------------------------------------------------------------

void __fastcall TfrmeLevelMeter::PaintBox1Paint(TObject *Sender)
{
    if ( !bmpBack_ || !bmpScale_ ) {
        CreateBitmaps1();
    }

    auto& PB = static_cast<TPaintBox&>( *Sender );
    auto& C = *PB.Canvas;

    C.Brush->Color = TColor( 0x3f1010 );    // Dark blue (a sort of)
    C.FillRect( offRegion_ );

    C.CopyRect( onRegion_, bmpBack_->Canvas, onRegion_ );

    if ( peakValue_.has_value() ) {
        auto dB = ValueTodB( *peakValue_ );
        auto Y = ComputeYPos( dB );
        C.Pen->Color = clYellow;
        C.Pen->Style = psSolid;
        C.MoveTo( offRegion_.Left, Y );
        C.LineTo( offRegion_.Right, Y );
    }

#if !defined( DONT_USE_SCALE_BITMAP )
    Gdiplus::Graphics g( C.Handle );
    g.DrawImage( bmpScale_.get(), 0, 0 );
#else
    C.Brush->Style = bsClear;
    C.Font->Assign( PB.Font );
    C.Font->Color = clWhite;
    auto TxtExtent = C.TextExtent( _D( "88" ) );
    auto TxtFmt = TTextFormat() << tfSingleLine << tfCenter << tfVerticalCenter;
    auto TickLen = ( PB.ClientRect.Width() - TxtExtent.cx ) / 2 - 1;

    for ( float dB = -6.0 ; dB > loLim_dB_ ; dB -= 6.0 ) {
        auto R = TRect{ PB.ClientRect.Left - 1, {}, PB.ClientRect.Right, {} };
        auto Text =
            Format( _D( "%.0f" ), static_cast<long double>( fabs( dB ) ) );
        auto Y = ComputeYPos( dB );

        C.Pen->Color = clBlack;
        C.MoveTo( PB.ClientRect.Left - 1, Y - 1 );
        C.LineTo( TickLen - 1, Y - 1 );
        C.MoveTo( PB.ClientRect.Right - TickLen - 1, Y - 1 );
        C.LineTo( PB.ClientRect.Right - 1, Y - 1 );

        C.Pen->Color = PB.Font->Color;
        C.MoveTo( PB.ClientRect.Left, Y );
        C.LineTo( TickLen, Y );
        C.MoveTo( PB.ClientRect.Right - TickLen, Y );
        C.LineTo( PB.ClientRect.Right, Y );

        R.Top = Y - TxtExtent.cy / 2 - 1;
        R.SetHeight( TxtExtent.cy );
        C.Font->Color = clBlack;
        C.TextRect( R, Text, TxtFmt );

        C.Font->Color = PB.Font->Color;
        R.Offset( 1, 1 );
        C.TextRect( R, Text, TxtFmt );
    }
#endif
}
//---------------------------------------------------------------------------

void __fastcall TfrmeLevelMeter::FrameResize(TObject *Sender)
{
    DestroyBitmaps();
    UpdateRegions();
    Invalidate();
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::UpdateRegions()
{
    auto dB = ValueTodB( Value );
    offRegion_ = onRegion_ = PaintBox1->ClientRect;
    auto Y = ComputeYPos( dB );
    offRegion_.Bottom = Y;
    onRegion_.Top = Y;
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::Reset()
{
    value_ = {};
    peakValue_.reset();
    UpdateRegions();
    Repaint();
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::CreateGradientFillBitmap()
{
    bmpBack_ = make_unique<TBitmap>( PaintBox1->ClientWidth, PaintBox1->ClientHeight );
    auto R = PaintBox1->ClientRect;
    auto D2DC = make_unique<TDirect2DCanvas>( bmpBack_->Canvas, R );
    _di_ID2D1LinearGradientBrush iBrush;
    TD2D1LinearGradientBrushProperties rLinear{
        { static_cast<float>( R.Left ), static_cast<float>( R.Top ) },
        { static_cast<float>( R.Left ), static_cast<float>( R.Bottom ) }
    };
    array<D2D1_GRADIENT_STOP, 3> arrGradientStop {
        D2D1_GRADIENT_STOP{ scaleLimitAt_, D2D1ColorF( clRed ) },
        D2D1_GRADIENT_STOP{ maxLevelAt_, D2D1ColorF( clYellow ) },
        D2D1_GRADIENT_STOP{ 1.0, D2D1ColorF( clGreen ) },
    };

    Winapi::D2d1::_di_ID2D1GradientStopCollection iGradientStops;
    D2DC->RenderTarget->CreateGradientStopCollection(
        arrGradientStop.data(), arrGradientStop.size(),
        D2D1_GAMMA_2_2,
        D2D1_EXTEND_MODE_CLAMP,
        iGradientStops
    );

    D2DC->RenderTarget->CreateLinearGradientBrush( rLinear, nullptr, iGradientStops, iBrush );
    D2DC->Brush->Handle = iBrush;
    D2DC->BeginDraw();
    D2DC->FillRect( R );
    D2DC->Font->Assign( PaintBox1->Font );
    D2DC->EndDraw();
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::DestroyBitmaps()
{
    bmpBack_.reset();
    bmpScale_.reset();;
}
//---------------------------------------------------------------------------

void TfrmeLevelMeter::CreateBitmaps1()
{
    CreateGradientFillBitmap();
#if !defined( DONT_USE_SCALE_BITMAP )
    CreateScaleBitmap();
#endif
}
//---------------------------------------------------------------------------

#if !defined( DONT_USE_SCALE_BITMAP )
void TfrmeLevelMeter::CreateScaleBitmap()
{
    auto& C = *PaintBox1->Canvas;
    C.Font->Assign( PaintBox1->Font );

    Gdiplus::Font f( C.Handle );

    bmpScale_ =
        make_unique<Bitmap>(
            PaintBox1->Width,
            PaintBox1->Height
        );
    Gdiplus::Graphics g( bmpScale_.get() );
    g.SetSmoothingMode( SmoothingModeAntiAlias );

    StringFormat sf;

    Pen BlackPen( Color::Black );
    SolidBrush BlackBrush( Color::Black );
    Pen WhitePen( Color::White );
    SolidBrush WhiteBrush( Color::White );

    for ( float dB = -6.0 ; dB > loLim_dB_ ; dB -= 6.0 ) {
        auto Text =
            Format( _D( "%.0f" ), static_cast<long double>( fabs( dB ) ) );
        auto Y = ComputeYPos( dB );

        Gdiplus::RectF BoundingBox;

        GdiplusCheck(
            g.MeasureString(
                Text.c_str(), -1, &f, Gdiplus::PointF{}, &BoundingBox
            )
        );

        auto TickLen = ( bmpScale_->GetWidth() - BoundingBox.Width ) / 2 - 1;

        GdiplusCheck(
            g.DrawLine(
                &BlackPen,
                static_cast<INT>( PaintBox1->ClientRect.Left - 1 ),
                static_cast<INT>( Y - 1 ),
                static_cast<INT>( PaintBox1->ClientRect.Left - 1 + TickLen ),
                static_cast<INT>( Y - 1 )
            )
        );
        GdiplusCheck(
            g.DrawLine(
                &BlackPen,
                static_cast<INT>( PaintBox1->ClientRect.Right - 1 - TickLen ),
                static_cast<INT>( Y - 1 ),
                static_cast<INT>( PaintBox1->ClientRect.Right - 1 ),
                static_cast<INT>( Y - 1 )
            )
        );

        auto TextX =
            ( PaintBox1->ClientRect.Left +
              PaintBox1->ClientRect.Right -
              BoundingBox.Width
            ) / 2.0;
        auto TextY = Y - BoundingBox.Height / 2;

        GdiplusCheck(
            g.DrawString(
                Text.c_str(),
                Text.Length(),
                &f,
                Gdiplus::PointF( TextX - 1, TextY - 1 ),
                &sf,
                &BlackBrush
            )
        );

        GdiplusCheck(
            g.DrawLine(
                &WhitePen,
                static_cast<INT>( PaintBox1->ClientRect.Left ),
                static_cast<INT>( Y ),
                static_cast<INT>( TickLen ),
                static_cast<INT>( Y )
            )
        );
        GdiplusCheck(
            g.DrawLine(
                &WhitePen,
                static_cast<INT>( PaintBox1->ClientRect.Right - TickLen ),
                static_cast<INT>( Y ),
                static_cast<INT>( PaintBox1->ClientRect.Right ),
                static_cast<INT>( Y )
            )
        );

        GdiplusCheck(
            g.DrawString(
                Text.c_str(),
                Text.Length(),
                &f,
                Gdiplus::PointF( TextX, TextY ),
                &sf,
                &WhiteBrush
            )
        );

    }
}
#endif
//---------------------------------------------------------------------------

