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

	void SetProcessId(int p) { procid = p; }
	void Update();

	class Mem
	{
	public:
		Mem();
		~Mem();
		size_t Populate(int pid);

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
	};


private:

	void DisplayGauge(HDC hdc) const;
	void DisplayBlobs(HDC hdc) const;

	void MemPaint(HDC hdc) const;
	COLORREF GetColour(std::vector<Mem::Region>::const_iterator& preg
		, std::vector<Mem::Region>::const_iterator rend, size_t base, size_t end) const;
	
	
	Mem mem;
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	HDC hMemDC;
	HBITMAP hBmp;
	HGDIOBJ hOldBmp;
	int procid;

	const int radius;
	const double dradius;
	const int width;

	size_t maxaddr;
};

#endif//MMPAINTER_H
