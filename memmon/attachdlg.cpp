// Copyright (c) 2007,2008 Charles Bailey
#include "stdafx.h"
#include "attachdlg.h"
#include "mmpainter.h"
#include "resource.h"
#include <sstream>

using std::wostringstream;

namespace
{

void PopulateProcessList(HWND listBox)
{
	DWORD pids[1024];
	DWORD pids_ret_size;

	if (::EnumProcesses(pids, sizeof pids, &pids_ret_size))
	{
		HANDLE hProc;
		TCHAR procname[1024];

		for (size_t i = (pids_ret_size / sizeof(DWORD)) - 1; i != (size_t)-1; --i)
		{
			hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
			if (hProc != NULL)
			{
				if (::GetModuleBaseName(hProc, NULL, procname, sizeof procname / sizeof(TCHAR)))
				{
					LRESULT lRes = ::SendMessage( listBox, LB_ADDSTRING, 0, LPARAM(procname) );
					if (lRes >= 0)
					{
						::SendMessage( listBox, LB_SETITEMDATA, lRes, pids[i] );
					}
				}
				else if (::GetProcessImageFileName(hProc, procname, sizeof procname / sizeof(TCHAR)))
				{
					LRESULT lRes = ::SendMessage( listBox, LB_ADDSTRING, 0, LPARAM(procname) );
					if (lRes >= 0)
					{
						::SendMessage( listBox, LB_SETITEMDATA, lRes, pids[i] );
					}
				}

			}
		}
	}
}

INT_PTR CALLBACK AttachProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = (INT_PTR)FALSE;
	switch (message)
	{

	case WM_COMMAND:
		{
			_TCHAR tmp[200] = { 0 };
			switch (LOWORD(wParam))
			{
			case IDC_PROCLIST:
				switch (HIWORD(wParam))
				{
				case LBN_SELCHANGE:
					{
						LRESULT lRes = ::SendMessage(HWND(lParam), LB_GETCURSEL, 0, 0);

						if (lRes >= 0)
						{
							lRes = ::SendMessage(HWND(lParam), LB_GETITEMDATA, lRes, 0);
							if (lRes >= 0)
							{
								wostringstream oss;
								oss << lRes;
								SetDlgItemText(hwndDlg, IDC_EDIT1, oss.str().c_str());
							}
						}
					}
					break;
				case LBN_DBLCLK:
					::SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
					break;
				}
				break;

			case IDOK:
				GetDlgItemText(hwndDlg, IDC_EDIT1, tmp, 200);
				try
				{
					reinterpret_cast< MMPainter* >( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) )->SetProcessId( _ttoi(tmp) );
				}
				catch( const std::exception& ex )
				{
					::MessageBoxA( hwndDlg, ex.what(), "Attach failed", MB_ICONINFORMATION | MB_OK );
				}
				// Drop through
			case IDCANCEL:
				EndDialog(hwndDlg, wParam);
				ret = (INT_PTR)TRUE;
			}
		}
		break;
	case WM_INITDIALOG:
		{
			::SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
			HWND hProcList = ::GetDlgItem(hwndDlg, IDC_PROCLIST);
			if (hProcList != NULL)
			{
				PopulateProcessList(hProcList);
			}
		}
		// Return true to set default focus
		ret = (INT_PTR)TRUE;
	}
	return ret;
}

}

bool AttachDialog::Run( HINSTANCE hInst, HWND hWnd, MMPainter* pPaint )
{
	return DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_ATTACH_DLOG), hWnd, AttachProc, reinterpret_cast< LPARAM >( pPaint ) ) == IDOK;
}
