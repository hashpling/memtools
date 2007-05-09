#include "stdafx.h"
#include "mmprefs.h"
#include <windows.h>

namespace
{
	const TCHAR prefs_subkey[] = _T("SOFTWARE\\hashpling.org\\memmon");
	const TCHAR prefs_cpu_k[] = _T("k");
	const TCHAR prefs_cpu_damper[] = _T("damper");
	const TCHAR prefs_cpu_use_cpu_count[] = _T("use_cpu_count");
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
	}
}
