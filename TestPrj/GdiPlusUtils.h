//---------------------------------------------------------------------------

#ifndef GdiPlusUtilsH
#define GdiPlusUtilsH

#include <Classes.hpp>
#include <SysUtils.hpp>

#include <GdiPlus.h>

#include <memory>

//---------------------------------------------------------------------------
namespace GdiplusUtils {
//---------------------------------------------------------------------------

class GdiPlusSessionManager {
public:
    GdiPlusSessionManager();
    ~GdiPlusSessionManager() /* throw() */;
    GdiPlusSessionManager( GdiPlusSessionManager const & ) = delete;
    GdiPlusSessionManager& operator=( GdiPlusSessionManager const & ) = delete;
private:
    Gdiplus::GdiplusStartupInput gdiplusStartupInput_;
    ULONG_PTR gdiplusToken_;
};

class EGdiplusExceptionBase : public Exception {
public:
    template<typename...A>
    EGdiplusExceptionBase( A&&... Args )
      : Exception( std::forward<A>( Args )... ) {}
};

class EGdiplusException : public EGdiplusExceptionBase {
public:
    __fastcall EGdiplusException( String Msg );
    __fastcall EGdiplusException( Gdiplus::Status Code );
};

extern String GetGdiplusStatusStrings( Gdiplus::Status Code );
extern void GdiplusCheck( Gdiplus::Status Code );
extern std::unique_ptr<Gdiplus::Image> LoadImage( String FileName );
extern Gdiplus::Color TColorToGdiplusColor( TColor Val );
extern Gdiplus::Color TColorToGdiplusColor( TColor Val, BYTE Alpha );
extern Gdiplus::StringAlignment AlignmentToGdiplusStringAlignment( TAlignment Val );
extern Gdiplus::StringAlignment VerticalAlignmentToGdiplusStringAlignment( TVerticalAlignment Val );
extern void DrawImage( Gdiplus::Graphics& g, Gdiplus::Image& Img,
                       Gdiplus::RectF const & DstRect, BYTE AlphaBlend,
                       bool Stretched = true, bool MantainAspectRatio = false );
extern Gdiplus::RectF VCLRectToGdiPlusRectF( TRect const & Rect );
extern void GetEncoderClsid( WCHAR const * Format, CLSID* Clsid );

//---------------------------------------------------------------------------
} // End of namespace GdiplusUtils
//---------------------------------------------------------------------------
#endif


