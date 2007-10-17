#ifndef MMPREFS_H
#define MMPREFS_H

// Copyright (c) 2007 Charles Bailey

#include <windows.h>

class MMPrefs
{
public:
	void Load();
	void Save() const;

	struct CPUPrefs
	{
		CPUPrefs();
		double k;
		double damper;
		bool use_cpu_count;
	};

	struct UIPrefs
	{
		UIPrefs();
		bool topmost;
	};

	const CPUPrefs& GetCPUPrefs() const { return cpuprf; }
	const UIPrefs& GetUIPrefs() const { return uiprf; }

	static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void RunDialog(HINSTANCE hInst, HWND hWnd);

	void DoSetDialogData(HWND hwndDlg);
	void DoReadDialogData(HWND hwndDlg);
private:
	CPUPrefs cpuprf;
	UIPrefs uiprf;
};

#endif//MMPREFS_H
