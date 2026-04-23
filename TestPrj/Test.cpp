//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

#if !defined( DONT_USE_SCALE_BITMAP )
# include "GdiPlusUtils.h"
#endif

//---------------------------------------------------------------------------
USEFORM("FormMain.cpp", frmMain);
USEFORM("FrameLevelMeter.cpp", frmeLevelMeter); /* TFrame: File Type */
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    try
    {
#if !defined( DONT_USE_SCALE_BITMAP )
		GdiplusUtils::GdiPlusSessionManager gdiplusmngr_;
#endif
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TfrmMain), &frmMain);
		Application->Run();
		while ( auto const Cnt = Screen->FormCount ) {
			delete Screen->Forms[Cnt - 1];
		}
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
