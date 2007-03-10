#ifndef MMPAINTER_H
#define MMPAINTER_H

#include <windows.h>

struct Mem;

class MMPainter
{
public:
	MMPainter();
	~MMPainter();

	void Paint(HDC hdc, PAINTSTRUCT* ps);

	void SetProcessId(int p) { procid = p; }
	void Update();

	class Mem
	{
	public:
		Mem();
		~Mem();
		void Populate(int pid);

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

		Region* head;
		Region* freelist;
	};


private:

	void DisplayGauge(HDC hdc) const;
	void DisplayBlobs(HDC hdc) const;

	void MemPaint(HDC hdc) const;
	COLORREF GetColour(Mem::Region** preg, size_t base, size_t end) const;
	
	
	Mem mem;
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	HDC hMemDC;
	HBITMAP hBmp;
	HGDIOBJ hOldBmp;
	int procid;
};

#endif//MMPAINTER_H
