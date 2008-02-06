#ifndef MMPREFS_H
#define MMPREFS_H

// Copyright (c) 2007 Charles Bailey

#include <windows.h>
#include "mmsource.h"

class MMPrefs
{
public:
	void Load();
	void Save() const;

	struct UIPrefs
	{
		UIPrefs();
		bool topmost;
	};

	const MemMon::CPUPrefs& GetCPUPrefs() const { return cpuprf; }
	const UIPrefs& GetUIPrefs() const { return uiprf; }

	static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void RunDialog(HINSTANCE hInst, HWND hWnd);

	void DoSetDialogData(HWND hwndDlg);
	void DoReadDialogData(HWND hwndDlg);
private:
	MemMon::CPUPrefs cpuprf;
	UIPrefs uiprf;
};

#endif//MMPREFS_H
