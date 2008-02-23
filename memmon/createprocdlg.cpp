#include "stdafx.h"
#include "createprocdlg.h"
#include "mmpainter.h"
#include "resource.h"

namespace
{

void CallRun( HWND hwndDlg )
{
	TCHAR command[ MAX_PATH ];

	if( GetDlgItemText( hwndDlg, IDC_EDIT_CMD, command, sizeof command / sizeof( TCHAR ) ) )
	{
		TCHAR workingdir[ MAX_PATH ];
		TCHAR *pwd = workingdir;
		TCHAR args[ MAX_PATH ];
		TCHAR *pag = args;

		if( GetDlgItemText( hwndDlg, IDC_EDIT_ARGS, args, sizeof args / sizeof( TCHAR ) ) == 0 )
			pag = NULL;

		if( GetDlgItemText( hwndDlg, IDC_EDIT_WD, workingdir, sizeof workingdir / sizeof( TCHAR ) ) == 0)
			pwd = NULL;

		try
		{
			reinterpret_cast< MMPainter* >( ::GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) )->Run( command, pwd, pag );
		}
		catch( const std::exception& ex )
		{
			::MessageBoxA( hwndDlg, ex.what(), "Exception", MB_ICONINFORMATION | MB_OK );
		}
	}
}

INT_PTR CALLBACK CreateProcProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	INT_PTR ret = (INT_PTR)FALSE;
	switch (message)
	{
	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
			case IDOK:
				CallRun( hwndDlg );
				// Drop through
			case IDCANCEL:
				EndDialog( hwndDlg, wParam );
				ret = (INT_PTR)TRUE;
		}
		break;

	case WM_INITDIALOG:
		::SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		ret = (INT_PTR)TRUE;
		break;
	}
	return ret;
}

}

void CreateProcDialog::Run( HINSTANCE hInst, HWND hWnd, MMPainter* pPaint, UINT_PTR& timerid )
{
	if( DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_CREATEPROC_DLOG), hWnd, CreateProcProc, reinterpret_cast< LPARAM >( pPaint ) ) )
	{
		timerid = SetTimer(hWnd, 1, 100, NULL);
	}
}
