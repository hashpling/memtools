#ifndef MMPREFS_H
#define MMPREFS_H

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

	const CPUPrefs& GetCPUPrefs() const { return cpuprf; }

	static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void RunDialog(HINSTANCE hInst, HWND hWnd);

	void DoSetDialogData(HWND hwndDlg);
	void DoReadDialogData(HWND hwndDlg);
private:
	CPUPrefs cpuprf;
};

#endif//MMPREFS_H
