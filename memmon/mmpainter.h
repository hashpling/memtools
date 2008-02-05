#ifndef MMPAINTER_H
#define MMPAINTER_H

// Copyright (c) 2007 Charles Bailey

#include <windows.h>
#include <vector>
#include <iosfwd>
#include <memory>
#include "mminfo.h"
#include "mmsource.h"

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

private:

	class ProcessSource : public MemMon::Source
	{
	public:
		ProcessSource( int pid );
		~ProcessSource();

		size_t Update( MemMon::MemoryMap& );
		double Poll( MMPrefs* pPrefs );
		double GetPos() const;

	private:
		HANDLE _proc;

		double actual_u;
		double actual_k;

		double ind_pos;
		double ind_vel;

		double last_poll;
	};

	void DisplayGauge(HDC hdc, bool bQuick) const;
	void DisplayBlobs(HDC hdc) const;
	void DisplayTotals(HDC hdc, int offset) const;

	void MemPaint(HDC hdc) const;
	COLORREF GetColour(std::vector<MemMon::Region>::const_iterator& preg
		, std::vector<MemMon::Region>::const_iterator rend, size_t base
		, size_t end) const;

	COLORREF GetBlobColour(const MemMon::FreeRegion& reg) const;

	MemMon::MemoryMap mem;
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	HDC hMemDC;
	HBITMAP hBmp;
	HGDIOBJ hOldBmp;
	RECT rsize;

	const int radius;
	const double dradius;
	const int width;

	size_t maxaddr;

	double next_update;
	double processor_count;

	MMPrefs* pPrefs;

	std::auto_ptr< MemMon::Source > _source;
};

#endif//MMPAINTER_H
