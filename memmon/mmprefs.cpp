// Copyright (c) 2007,2008 Charles Bailey

#include "stdafx.h"
#include "mmprefs.h"
#include <windows.h>
#include "resource.h"
#include <sstream>
#include <string>

using std::wostringstream;
using std::wistringstream;
using std::wstring;
using std::ios_base;
	
namespace
{
	const TCHAR prefs_subkey[] = _T("SOFTWARE\\hashpling.org\\memmon");
	const TCHAR prefs_cpu_k[] = _T("k");
	const TCHAR prefs_cpu_damper[] = _T("damper");
	const TCHAR prefs_cpu_use_cpu_count[] = _T("use_cpu_count");
	const TCHAR prefs_ui_topmost[] = _T("top_most");
}

MMPrefs::UIPrefs::UIPrefs()
// Global defaults
: topmost(false)
{
}

void MMPrefs::Load()
{
	HKEY hk;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwcbData;

		dwcbData = sizeof(cpuprf.k);
		::RegQueryValueEx(hk, prefs_cpu_k, NULL, &dwType, (LPBYTE)&cpuprf.k, &dwcbData);

		dwcbData = sizeof(cpuprf.damper);
		::RegQueryValueEx(hk, prefs_cpu_damper, NULL, &dwType, (LPBYTE)&cpuprf.damper, &dwcbData);

		dwcbData = sizeof(cpuprf.use_cpu_count);
		::RegQueryValueEx(hk, prefs_cpu_use_cpu_count, NULL, &dwType, (LPBYTE)&cpuprf.use_cpu_count, &dwcbData);

		dwcbData = sizeof(uiprf.topmost);
		::RegQueryValueEx(hk, prefs_ui_topmost, NULL, &dwType, (LPBYTE)&uiprf.topmost, &dwcbData);

		::RegCloseKey( hk );
	}
}

void MMPrefs::Save() const
{
	HKEY hk;
	DWORD dwDisposition;
	if (::RegCreateKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, &dwDisposition) == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwcbData;

		dwType = REG_QWORD;
		dwcbData = sizeof(cpuprf.k);
		::RegSetValueEx(hk, prefs_cpu_k, 0, dwType, (const BYTE*)&cpuprf.k, dwcbData);

		dwcbData = sizeof(cpuprf.damper);
		::RegSetValueEx(hk, prefs_cpu_damper, 0, dwType, (const BYTE*)&cpuprf.damper, dwcbData);

		dwType = REG_BINARY;
		dwcbData = sizeof(cpuprf.use_cpu_count);
		::RegSetValueEx(hk, prefs_cpu_use_cpu_count, 0, dwType, (const BYTE*)&cpuprf.use_cpu_count, dwcbData);

		dwcbData = sizeof(uiprf.topmost);
		::RegSetValueEx(hk, prefs_ui_topmost, 0, dwType, (const BYTE*)&uiprf.topmost, dwcbData);

		::RegCloseKey( hk );
	}
}

void MMPrefs::RunDialog(HINSTANCE hInst, HWND hWnd)
{
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREFS_DLOG), hWnd, DialogProc, (LPARAM)this) == IDOK)
	{
		Save();
	}
}

INT_PTR CALLBACK MMPrefs::DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	MMPrefs* pPrefs;
	INT_PTR ret = (INT_PTR)FALSE;
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			pPrefs = reinterpret_cast<MMPrefs*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			pPrefs->DoReadDialogData(hwndDlg);
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			ret = (INT_PTR)TRUE;
		}
		break;
	case WM_INITDIALOG:
		// Return true to set default focus
		pPrefs = reinterpret_cast<MMPrefs*>(lParam);
		pPrefs->DoSetDialogData(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pPrefs);
		ret = (INT_PTR)TRUE;
	}
	return ret;
}

void MMPrefs::DoSetDialogData(HWND hwndDlg)
{
	HWND hItem;
	wstring emptystring;
	wostringstream oss;

	hItem = GetDlgItem(hwndDlg, IDC_EDIT_K);

	if (hItem != NULL)
	{
		oss << cpuprf.k;
		SetWindowText( hItem, oss.str().c_str() );
		oss.str(emptystring);
	}

	hItem = GetDlgItem(hwndDlg, IDC_EDIT_DAMPING);

	if (hItem != NULL)
	{
		oss << cpuprf.damper;
		SetWindowText( hItem, oss.str().c_str() );
		oss.str(emptystring);
	}

	CheckDlgButton(hwndDlg, IDC_CHECK_CPUCOUNT, cpuprf.use_cpu_count ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hwndDlg, IDC_CHECK_TOPMOST, uiprf.topmost ? BST_CHECKED : BST_UNCHECKED);
}

namespace
{

bool GetWindowTextIntoIstream(HWND hItem, wistringstream& iss)
{
	bool success = false;
	if (hItem != NULL)
	{
		int n = GetWindowTextLength( hItem ) + 1;

		// sanity 2048 is huge in this context
		if (n > 0 && n < 2048)
		{
			TCHAR* tmp = new TCHAR[n];
			GetWindowText( hItem, tmp, n );

			// sanity - determined as not necessary
			// tmp[n - 1] = 0;

			iss.str( tmp );
			iss.rdbuf()->pubseekpos(0, ios_base::in);
			iss.clear();
			delete[] tmp;

			success = true;
		}
	}
	return success;
}

}

void MMPrefs::DoReadDialogData(HWND hwndDlg)
{
	HWND hItem;
	wistringstream iss;

	hItem = GetDlgItem(hwndDlg, IDC_EDIT_K);

	if (hItem != NULL && GetWindowTextIntoIstream(hItem, iss))
	{
		iss >> cpuprf.k;
	}

	hItem = GetDlgItem(hwndDlg, IDC_EDIT_DAMPING);

	if (hItem != NULL && GetWindowTextIntoIstream(hItem, iss))
	{
		iss >> cpuprf.damper;
	}

	UINT ischecked;
	
	ischecked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_CPUCOUNT);

	if (ischecked == BST_CHECKED)
	{
		cpuprf.use_cpu_count = true;
	}
	else if (ischecked == BST_UNCHECKED)
	{
		cpuprf.use_cpu_count = false;
	}

	ischecked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_TOPMOST);

	if (ischecked == BST_CHECKED)
	{
		uiprf.topmost = true;
	}
	else if (ischecked == BST_UNCHECKED)
	{
		uiprf.topmost = false;
	}
}

