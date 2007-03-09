#include "stdafx.h"
#include "mmpainter.h"
#include <cmath>

using std::cos;
using std::sin;

const int radius = 100;
const double dradius = radius;
const int width = 60;

MMPainter::MMPainter()
{
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	hWBrush = CreateSolidBrush(RGB(255, 255, 255));
	hPen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
}

MMPainter::~MMPainter()
{
	DeleteObject(hPen);
	DeleteObject(hWBrush);
	DeleteObject(hBrush);
}

void MMPainter::Paint(HDC hdc) const
{
	//HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
	//Pie(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, radius, 0);
	//SelectObject(hdc, oldBrush);
	DisplayGauge(hdc);
	HGDIOBJ oldPen = SelectObject(hdc, hPen);
	HGDIOBJ oldBrush = SelectObject(hdc, hWBrush);
	int ww = (radius << 1) - width;
	Pie(hdc, width, width, ww, ww, ww, radius, width, radius);
	SelectObject(hdc, oldPen);
	Arc(hdc, width, width, ww, ww, ww, radius, width, radius);
	Arc(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, 0, radius);
	SelectObject(hdc, oldBrush);
}

struct Mem
{
	size_t min;
	size_t max;

	struct Region
	{
		size_t base;
		size_t size;
		int type;
	};

	Region* head;
};

void MMPainter::DisplayGauge(HDC hdc) const
{
	double pi = acos(-1.0);
	double dstart = pi;
	double ddiff = dstart / (dradius/2.0);
	double dnext;

	double addrmax = double(0x80000000u);

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	for (double d = dstart; d > ddiff; d = dnext)
	{
		dnext = d - ddiff;

		size_t base = size_t((pi - d) * addrmax);
		size_t end = size_t((pi - dnext) * addrmax);

		COLORREF c = RGB((rand() * 256) / RAND_MAX
					, (rand() * 256) / RAND_MAX, (rand() * 256) / RAND_MAX);
		SetDCBrushColor(hdc, c);
		SetDCPenColor(hdc, c);
		int x1 = radius + int(dradius * cos(dnext));
		int y1 = radius - int(dradius * sin(dnext));
		int x2 = radius + int(dradius * cos(d));
		int y2 = radius - int(dradius * sin(d));
		Pie(hdc, 0, 0, radius << 1, radius << 1, x1, y1, x2, y2);

	}
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
}
