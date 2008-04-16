// Copyright (c) 2007,2008 Charles Bailey
#include "stdafx.h"
#include "filedialog.h"
#include <windows.h>
#include <shlobj.h>

namespace MemMon
{
namespace Win
{

#if _MSC_VER < 1500
typedef LPITEMIDLIST IdListPtr;
#else
typedef PIDLIST_ABSOLUTE IdListPtr;
#endif

namespace
{

void SetupCommon( OPENFILENAMEA& ofn, HWND hwnd, char* fnamebuf )
{
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = fnamebuf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.pvReserved = NULL;
	ofn.dwReserved = NULL;
	ofn.FlagsEx = 0;
}

}

bool RunSaveFileDialog( HWND hwnd, char* fnamebuf )
{
	fnamebuf[0] = 0;

	OPENFILENAMEA ofn;

	SetupCommon( ofn, hwnd, fnamebuf );
	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOTESTFILECREATE | OFN_OVERWRITEPROMPT;

	return ::GetSaveFileNameA(&ofn) == TRUE;
}

bool RunOpenFileDialog( HWND hwnd, char* fnamebuf )
{
	fnamebuf[0] = 0;

	OPENFILENAMEA ofn;

	SetupCommon( ofn, hwnd, fnamebuf );
	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOTESTFILECREATE | OFN_FILEMUSTEXIST;

	return ::GetOpenFileNameA(&ofn) == TRUE;
}

bool RunSelectDirDialog( HWND hwnd, char* fnamebuf )
{
	BROWSEINFO bi;

	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = NULL;
	// BIF_RETURNONLYFSDIRS doesn seem to work with BIF_USENEWUI
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	IdListPtr pidlabs = ::SHBrowseForFolder( &bi );

	bool success = false;

	if( pidlabs != NULL )
	{
		success = ::SHGetPathFromIDListA( pidlabs, fnamebuf ) == TRUE;
		::CoTaskMemFree( pidlabs );
	}

	return success;
}

}
}
