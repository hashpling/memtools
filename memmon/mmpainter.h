#ifndef MMPAINTER_H
#define MMPAINTER_H

#include <windows.h>
#include <vector>

struct Mem;

class MMPainter
{
public:
	MMPainter(int r);
	~MMPainter();

	void Paint(HDC hdc, PAINTSTRUCT* ps);

	void SetProcessId(int p);
	void Update();

	const RECT& GetRect() const { return rsize; }

	class Mem
	{
	public:
		Mem();
		~Mem();
		size_t Populate(HANDLE hProc);

		size_t min;
		size_t max;

		struct FreeRegion
		{
			size_t base;
			size_t size;
		};

		struct Region : public FreeRegion
		{
			int type;
		};

		std::vector<Region> blocklist;
		std::vector<FreeRegion> freelist;

		size_t total_free;
		size_t total_commit[4];
		size_t total_reserve[4];
	};

	class CPUPerf
	{
	public:
		CPUPerf();

		double Poll(HANDLE hProc);

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
	COLORREF GetColour(std::vector<Mem::Region>::const_iterator& preg
		, std::vector<Mem::Region>::const_iterator rend, size_t base
		, size_t end) const;

	COLORREF GetBlobColour(const Mem::FreeRegion& reg) const;
	
	Mem mem;
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
};

#endif//MMPAINTER_H
