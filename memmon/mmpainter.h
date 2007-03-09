#ifndef MMPAINTER_H
#define MMPAINTER_H

#include <windows.h>

class MMPainter
{
public:
	MMPainter();
	~MMPainter();

	void Paint(HDC hdc) const;

	void DisplayGauge(HDC hdc) const;

private:
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
};

#endif//MMPAINTER_H
