#include "stdafx.h"
#include "mmpainter.h"
#include <cmath>
#include <vector>
#include <algorithm>

using std::cos;
using std::sin;
using std::vector;
using std::copy;

MMPainter::MMPainter(int r)
: hMemDC(NULL), hBmp(NULL), hOldBmp(NULL)
, radius(r), dradius(r), width(r * 3 /  5), maxaddr(0x1000000u)
{
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	hWBrush = CreateSolidBrush(RGB(255, 255, 255));
	hPen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
}

MMPainter::~MMPainter()
{
	if (hBmp != NULL)
	{
		if (hMemDC != NULL)
		{
			SelectObject(hMemDC, hOldBmp);
		}
		DeleteObject(hBmp);
	}
	if (hMemDC != NULL)
	{
		DeleteDC(hMemDC);
	}

	DeleteObject(hPen);
	DeleteObject(hWBrush);
	DeleteObject(hBrush);
}

void MMPainter::Paint(HDC hdc, PAINTSTRUCT* ps)
{
	if (hMemDC == NULL)
	{
		hMemDC = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, radius << 1, radius + 180);
		hOldBmp = SelectObject(hMemDC, hBmp);

		SetBkColor(hMemDC, RGB(255, 255, 255));
	}

	int x, y, w, h;
	x = max(ps->rcPaint.left, 0);
	y = max(ps->rcPaint.top, 0);
	w = min(ps->rcPaint.right - x, (radius << 1) - x);
	h = min(ps->rcPaint.bottom - y, (radius + 180) - y);
	BitBlt(hdc, x, y, w, h, hMemDC, x, y, SRCCOPY);
}

void MMPainter::MemPaint(HDC hdc) const
{
	RECT r = { 0, 0, radius << 1, radius + 180 };
	FillRect(hdc, &r, hWBrush);
	DisplayGauge(hdc);
	HGDIOBJ oldPen = SelectObject(hdc, hPen);
	HGDIOBJ oldBrush = SelectObject(hdc, hWBrush);
	int ww = (radius << 1) - width;
	Pie(hdc, width, width, ww, ww, ww, radius, width, radius);
	SelectObject(hdc, oldPen);
	Arc(hdc, width, width, ww, ww, ww, radius, width, radius);
	Arc(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, 0, radius);
	SelectObject(hdc, oldBrush);
	DisplayBlobs(hdc);
}


COLORREF MMPainter::GetColour(std::vector<Mem::Region>::const_iterator& reg
							, std::vector<Mem::Region>::const_iterator rend
							, size_t base, size_t end) const
{
	COLORREF c = RGB(0, 0, 0);

	if (reg != rend)
	{
		while (reg != rend && (reg->base + reg->size) < base)
		{
			++reg;
		}

		size_t tmp = 0, tmp2 = 0;
		int max_type = 0;

		std::vector<Mem::Region>::const_iterator oreg = reg;
		while (reg != rend && reg->base < end)
		{
			size_t sbegin = max(base, reg->base);
			size_t send = min(end, reg->base + reg->size);

			tmp += (send - sbegin) * reg->type;
			tmp2 += (send - sbegin) * max(reg->type, 1);

			if (reg->type > max_type) max_type = reg->type;

			oreg = reg;
			++reg;
		}
		// Reset back to the last interval so that the overlaps match up
		reg = oreg;

		if (max_type > 0) tmp = tmp2;

		if (tmp > end - base)
		{
			c = RGB(255, 255 - ((tmp - (end - base)) * 255) / (end - base), 0);
		}
		else
		{
			c = RGB((tmp * 255) / (end - base), 255, 0);
		}
	}

	return c;
}

void MMPainter::DisplayGauge(HDC hdc) const
{
	double pi = acos(-1.0);
	double dstart = pi;
	double ddiff = dstart / (dradius*2.0);
	double dnext;

	double addrmax = double(maxaddr);

	vector<Mem::Region>::const_iterator pReg = mem.blocklist.begin();

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	for (double d = dstart; d > ddiff; d = dnext)
	{
		dnext = d - ddiff;

		size_t base = size_t((pi - d) * addrmax / pi);
		size_t end = size_t((pi - dnext) * addrmax / pi);

		COLORREF c = GetColour(pReg, mem.blocklist.end(), base, end);
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

void MMPainter::DisplayBlobs(HDC hdc) const
{
/*	if (!mem.freelist.empty())
	{
		HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
		HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
		const double scale = double(0x1000000);
		const double scale2 = log(double(maxaddr) / scale);

		SetDCPenColor(hdc, RGB(0, 0, 0));

		for (int j = 0; j < int(mem.freelist.size()); ++j)
		{
			const Mem::FreeRegion& r = mem.freelist[j];
			if (r.size == 0) break;
			BYTE c = BYTE(255 * double(r.size + r.base)/double(maxaddr));
			SetDCBrushColor(hdc, RGB(255 - c, c, 255));

			int size = int(log(double(r.size) / scale) / scale2 * dradius);
			if (size > 0)
			{
				Ellipse(hdc, radius - size, radius + 20 * j, radius + size
													, radius + 20 * (j + 1));
			}
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}*/
	int currentline = 1;
	int currentpos = 0;

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, RGB(0, 0, 0));

	for (vector<Mem::FreeRegion>::const_iterator k = mem.freelist.begin();
			k != mem.freelist.end(); ++k)
	{
		double width = dradius * 8.0 * double(k->size) / double(maxaddr);
		int nwidth = int(width);

		if (nwidth < 6) break;

		if (currentpos > 0 && nwidth + currentpos > (radius << 1))
		{
			++currentline;
			currentpos = 0;
		}

		if (nwidth > (radius << 2)) nwidth = radius << 2;

		int c = int(511.0 * double(k->size + k->base)/double(maxaddr));
		int r, g, b;
		if (c < 256)
		{
			r = 255;
			g = 255 - c;
			b = c;
		}
		else
		{
			r = 511 - c;
			g = c - 256;
			b = 255;
		}
		//BYTE c = BYTE(255 * double(k->size + k->base)/double(maxaddr));
		SetDCBrushColor(hdc, RGB(r, g, b));
		Ellipse(hdc, currentpos, radius + 20 * currentline, currentpos + nwidth
											, radius + 20 * (currentline + 1));

		currentpos += nwidth + 1;

	}

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
}

void MMPainter::Update()
{
	maxaddr = mem.Populate(procid);
	if (hMemDC != NULL)
	{
		MemPaint(hMemDC);
	}
}

MMPainter::Mem::Mem()
{
}

MMPainter::Mem::~Mem()
{
}

size_t MMPainter::Mem::Populate(int pid)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	size_t max_addr = (size_t)sysinfo.lpMaximumApplicationAddress;

	Region r;

	freelist.resize(50);
	blocklist.clear();

	for (vector<FreeRegion>::iterator i = freelist.begin();
									i != freelist.end(); ++i)
	{
		i->size = 0;
	}

	MEMORY_BASIC_INFORMATION meminfo;

	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (hProc != NULL)
	{
		for (char* p = (char*)sysinfo.lpMinimumApplicationAddress;
			p < (char*)sysinfo.lpMaximumApplicationAddress;
			p += sysinfo.dwPageSize)
		{
			VirtualQueryEx(hProc, p, &meminfo, sizeof(meminfo));

			if (p != meminfo.BaseAddress) break;

			r.base = (size_t)meminfo.BaseAddress;
			r.size = meminfo.RegionSize;
			switch (meminfo.State)
			{
			case MEM_FREE:
				r.type = 0;
				break;
			case MEM_RESERVE:
				r.type = 1;
				break;
			case MEM_COMMIT:
			default:
				r.type = 2;
			}

			blocklist.push_back(r);

			if (meminfo.State == MEM_FREE)
			{
				if (freelist.back().size < meminfo.RegionSize)
				{
					int j;

					for(j = int(freelist.size()) - 2; j >= 0; --j)
					{
						if (freelist[j].size >= meminfo.RegionSize) break;
						freelist[j+1].size = freelist[j].size;
						freelist[j+1].base = freelist[j].base;
					}

					freelist[j+1].size = meminfo.RegionSize;
					freelist[j+1].base = size_t(meminfo.BaseAddress);
				}
			}

			if (meminfo.RegionSize > 0)
			{
				p += (meminfo.RegionSize - sysinfo.dwPageSize);
			}
		}

		CloseHandle(hProc);

		max_addr = (size_t)meminfo.BaseAddress + meminfo.RegionSize;
	}

	return max_addr;
}
