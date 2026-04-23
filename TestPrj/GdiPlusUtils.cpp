//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <vector>

#include "GdiPlusUtils.h"

using std::make_unique;
using std::unique_ptr;
using std::vector;

using Gdiplus::GdiplusStartup;
using Gdiplus::GdiplusShutdown;
using Gdiplus::Status;
using Gdiplus::StringAlignment;
using Gdiplus::StringAlignmentNear;
using Gdiplus::StringAlignmentCenter;
using Gdiplus::StringAlignmentFar;
using Gdiplus::Image;
using Gdiplus::RectF;
using Gdiplus::ColorMatrixFlagsDefault;
using Gdiplus::ColorAdjustTypeDefault;
using Gdiplus::ColorMatrix;
using Gdiplus::ImageAttributes;
using Gdiplus::GetImageEncodersSize;
using Gdiplus::ImageCodecInfo;

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
namespace GdiplusUtils {
//---------------------------------------------------------------------------

GdiPlusSessionManager::GdiPlusSessionManager()
{
    GdiplusStartup( &gdiplusToken_, &gdiplusStartupInput_, NULL );
}
//---------------------------------------------------------------------------

GdiPlusSessionManager::~GdiPlusSessionManager()
{
    try {
        GdiplusShutdown( gdiplusToken_ );
    }
    catch ( ... ) {
    }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall EGdiplusException::EGdiplusException( String Msg )
    : EGdiplusExceptionBase( _D( "GDI+ error: %s" ), ARRAYOFCONST(( Msg )) )
{
}
//---------------------------------------------------------------------------

__fastcall EGdiplusException::EGdiplusException( Gdiplus::Status Code )
    : EGdiplusExceptionBase( _D( "GDI+ error: %s" ), ARRAYOFCONST(( GetGdiplusStatusStrings( Code ) )) )
{
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static String FormatOSErrorMessage( DWORD ErrCode );

void GdiplusCheck( Gdiplus::Status Code )
{
    switch ( Code ) {
        case Status::Ok:
            return;
        case Status::Win32Error:
            throw EGdiplusException(
                Format(
                    _D( "%s: %s" ),
                    GetGdiplusStatusStrings( Code ),
                    FormatOSErrorMessage( GetLastError() )
                )
            );
        default:
            throw EGdiplusException( Code );
    }
}
//---------------------------------------------------------------------------

String GetGdiplusStatusStrings( Gdiplus::Status Code ) {
	static constexpr LPCTSTR Strs[] = {
      _D( "Ok" ),
      _D( "Generic Error" ),
      _D( "Invalid Parameter" ),
      _D( "Out Of Memory" ),
      _D( "Object Busy" ),
      _D( "Insufficient Buffer" ),
      _D( "Not Implemented" ),
      _D( "Win32 Error" ),
      _D( "Wrong State" ),
      _D( "Aborted" ),
      _D( "File Not Found" ),
      _D( "Value Overflow" ),
      _D( "Access Denied" ),
      _D( "Unknown Image Format" ),
      _D( "Font Family Not Found" ),
      _D( "Font Style Not Found" ),
      _D( "Not TrueType Font" ),
      _D( "Unsupported Gdi+ Version" ),
      _D( "Gdi+ Not Initialized" ),
      _D( "Property Not Found" ),
      _D( "Property Not Supported" ),
      _D( "Profile Not Found" ),
    };

    if ( Code < static_cast<int>( sizeof Strs / sizeof *Strs ) ) {
        return Strs[Code];
    }
    else {
        return Format( _D( "Unknown (code %d)" ), ARRAYOFCONST(( Code )) );
    }
}
//---------------------------------------------------------------------------

class TEOSErrorLocalFree {
public:
    explicit TEOSErrorLocalFree( PVOID Buffer )
        : Buffer_( Buffer ) {}
    ~TEOSErrorLocalFree() { LocalFree( Buffer_ ); }
    TEOSErrorLocalFree( TEOSErrorLocalFree const & ) = delete;
    TEOSErrorLocalFree operator=( TEOSErrorLocalFree const & ) = delete;
private:
    PVOID Buffer_;
};

String FormatOSErrorMessage( DWORD ErrCode )
{
    PVOID Buffer;

    DWORD Ret = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                0, ErrCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<PTCHAR>( &Buffer ), 0,
                0
          );
    if ( !Ret ) {
        return _D( "FormatOSErrorMessage failed!" );
    }

    TEOSErrorLocalFree LocalFreeObj( Buffer );

    return String( static_cast<PTCHAR>( Buffer ) );
}
//---------------------------------------------------------------------------

unique_ptr<Gdiplus::Image> LoadImage( String FileName )
{
    if ( FileExists( FileName ) ) {
        auto NewImage = make_unique<Gdiplus::Image>( FileName.c_str() );
        if ( NewImage ) {
            GdiplusCheck( NewImage->GetLastStatus() );
        }
        return NewImage;
    }
    else {
        throw EGdiplusException(
            Format( _D( "Can't load the image \"%s\"" ), FileName )
        );
    }
}
//---------------------------------------------------------------------------

Gdiplus::Color TColorToGdiplusColor( TColor Val )
{
    return Gdiplus::Color(
        ( Val >> 24 ) & 0xFF,
        Val & 0xFF,
        ( Val >> 8 ) & 0xFF,
        ( Val >> 16 ) & 0xFF
    );
}
//---------------------------------------------------------------------------

Gdiplus::Color TColorToGdiplusColor( TColor Val, BYTE Alpha )
{
    return Gdiplus::Color(
        Alpha, Val & 0xFF, ( Val >> 8 ) & 0xFF, ( Val >> 16 ) & 0xFF
    );
}
//---------------------------------------------------------------------------

StringAlignment AlignmentToGdiplusStringAlignment( TAlignment Val )
{
    switch ( Val ) {
        case taLeftJustify:
            return StringAlignmentNear;
        case taRightJustify:
            return StringAlignmentFar;
        case taCenter:
            return StringAlignmentCenter;
    }
    return StringAlignmentCenter;
}
//---------------------------------------------------------------------------

StringAlignment VerticalAlignmentToGdiplusStringAlignment( TVerticalAlignment Val )
{
    switch ( Val ) {
        case taAlignTop:
            return StringAlignmentNear;
        case taAlignBottom:
            return StringAlignmentFar;
        case taVerticalCenter:
            return StringAlignmentCenter;
    }
    return StringAlignmentCenter;
}
//---------------------------------------------------------------------------

void DrawImage( Gdiplus::Graphics& g, Gdiplus::Image& Img, Gdiplus::RectF const & DstRect,
                BYTE AlphaBlend, bool Stretched, bool MantainAspectRatio )
{
    ImageAttributes imgAttrs;

    ImageAttributes* pImgAttrs = 0; //&imgAttrs;

    // Con GDI+ 1.0, quelle di XP per intenderci, se si applicano
    // gli attributi ad una immagine (Gdiplus::ImageAttributes), il metodo
    // Graphics::DrawImage se ne sbatte dell'ultima chiamata al metodo
    // Image::SelectActiveFrame ed usa sempre il frame 0. Quindi, se si
    // vogliono far funzionare - ad esempio - le multigif, si deve evitare
    // di applicare i suddetti attributi, passando pImgAttrs == null al metodo
    // Graphics::DrawImage utilizzato pi� in basso. Siccome gli attributi -
    // in questa applicazione - si usano solo per generare la traslucenza
    // (alphablend), se quest'ultima non � necessaria, si passa null come
    // pImgAttrs. Con GDI+ 1.1 questo problema sembra non esistere (su seven,
    // ad esempio, che come Vista ha le 1.1, funziona a dovere).
    // Morale: se si vogliono vedere le gif animate su XP non si deve dare
    // la trasparenza alle immagini

    if ( AlphaBlend < 255 ) {
        static ColorMatrix const UnityColMat = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };

        ColorMatrix ColMat( UnityColMat );

        ColMat.m[3][3] = (float)AlphaBlend / 255.0;

        imgAttrs.SetColorMatrix( &ColMat, ColorMatrixFlagsDefault, ColorAdjustTypeDefault );

        pImgAttrs = &imgAttrs;

    }

    float ImgWidth = Img.GetWidth();
    float ImgHeight = Img.GetHeight();

    if ( Stretched || MantainAspectRatio ) {
        float DstWidth = DstRect.Width;
        float DstHeight = DstRect.Height;
        if ( MantainAspectRatio && ImgWidth > 0 && ImgHeight > 0 && DstWidth > 0 && DstHeight > 0 ) {
            float const Rs = ImgWidth / ImgHeight;
            float const Rd = DstWidth / DstHeight;

            String Msg;

            float NewDstWidth;
            float NewDstHeight;
            float DX = DstRect.GetLeft();
            float DY = DstRect.GetTop();
            float k = Rs / Rd;

            if ( Rs > Rd ) {
                NewDstWidth = DstWidth;
                NewDstHeight = DstHeight / k;

                DY += ( DstHeight - NewDstHeight ) / 2.0F;
            }
            else if ( Rs < Rd ) {
                NewDstWidth = DstWidth * k;
                NewDstHeight = DstHeight;

                DX += ( DstWidth - NewDstWidth ) / 2.0F;

            }
            else {
                NewDstWidth = DstWidth;
                NewDstHeight = DstHeight;
            }

            Gdiplus::RectF DRect( DX, DY, NewDstWidth, NewDstHeight );

            g.DrawImage(
                &Img, DRect, 0.0F, 0.0F, ImgWidth, ImgHeight,
                Gdiplus::UnitPixel, pImgAttrs, 0, 0
            );

        }
        else
            g.DrawImage(
                &Img, DstRect, 0.0F, 0.0F, ImgWidth, ImgHeight,
                Gdiplus::UnitPixel, pImgAttrs, 0, 0
            );
    }
    else {
        Gdiplus::RectF DRect( DstRect.GetLeft(), DstRect.GetTop(), ImgWidth, ImgHeight );
        g.DrawImage(
            &Img, DRect, 0.0F, 0.0F, ImgWidth, ImgHeight,
            Gdiplus::UnitPixel, pImgAttrs, 0, 0
        );
    }
}
//---------------------------------------------------------------------------

Gdiplus::RectF VCLRectToGdiPlusRectF( TRect const & Rect )
{
    return Gdiplus::RectF(
        static_cast<float>( Rect.Left ),
        static_cast<float>( Rect.Top ),
        static_cast<float>( Rect.Width() ),
        static_cast<float>( Rect.Height() )
    );
}
//---------------------------------------------------------------------------

void GetEncoderClsid( WCHAR const * Format, CLSID* Clsid )
{
    UINT Num = 0;
    UINT Size = 0;

    GdiplusCheck( GetImageEncodersSize( &Num, &Size ) );

    if ( Size == 0 )
        throw Exception( "GetImageEncodersSize() failure!" );

    vector<char> ImageCodecInfoBuffer( Size );

    ImageCodecInfo* const ImgCodecInfo =
        reinterpret_cast<ImageCodecInfo*>( &ImageCodecInfoBuffer[0] );

    GdiplusCheck( GetImageEncoders( Num, Size, ImgCodecInfo ) );

    for ( UINT j = 0 ; j < Num ; ++j )
       if ( !wcscmp( ImgCodecInfo[j].MimeType, Format ) ) {
           *Clsid = ImgCodecInfo[j].Clsid;
           break;
       }
}
//---------------------------------------------------------------------------

// LoadImageFromStream intentionally dropped from the WaveOut port: it relied
// on calling TInterfacedObject::_AddRef / _Release, which are protected in
// RAD Studio 13's RTL. FrameLevelMeter does not need it.

//---------------------------------------------------------------------------
} // End of namespace GdiplusUtils
//---------------------------------------------------------------------------




