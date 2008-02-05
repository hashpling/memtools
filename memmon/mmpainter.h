#ifndef MMPAINTER_H
#define MMPAINTER_H

// Copyright (c) 2007 Charles Bailey

#include <windows.h>
#include <vector>
#include <iosfwd>
#include "mminfo.h"

class MMPrefs;

class MMPainter
{
public:
	MMPainter(int r, MMPrefs* p);
	~MMPainter();

	void Paint(HDC hdc, PAINTSTRUCT* ps);

	void SetProcessId(int p);
	void Update();

	const RECT& GetRect() const { return rsize; }

	void Snapshot(HWND hwnd) const;
	void Read(HWND hwnd);

	class CPUPerf
	{
	public:
		CPUPerf();

		double Poll(HANDLE hProc, MMPrefs* pPrefs);

		double GetPos() const;

	private:
		double actual_u;
		double actual_k;

		double ind_pos;
		double ind_vel;

		double last_poll;
	};

private:

	void DisplayGauge(HDC hdc, bool bQuick) const;
	void DisplayBlobs(HDC hdc) const;
	void DisplayTotals(HDC hdc, int offset) const;

	void MemPaint(HDC hdc) const;
	COLORREF GetColour(std::vector<MMInfo::Region>::const_iterator& preg
		, std::vector<MMInfo::Region>::const_iterator rend, size_t base
		, size_t end) const;

	COLORREF GetBlobColour(const MMInfo::FreeRegion& reg) const;
	
	MMInfo::MemoryMap mem;
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	HDC hMemDC;
	HBITMAP hBmp;
	HGDIOBJ hOldBmp;
	RECT rsize;

	HANDLE hProc;

	const int radius;
	const double dradius;
	const int width;

	size_t maxaddr;

	CPUPerf cpup;

	double next_update;
	double processor_count;

	MMPrefs* pPrefs;
};

#endif//MMPAINTER_H
