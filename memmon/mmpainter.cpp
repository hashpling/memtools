#include "stdafx.h"
#include "mmpainter.h"
#include <cmath>
#include <vector>
#include <algorithm>

using std::cos;
using std::sin;
using std::vector;
using std::copy;

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


COLORREF MMPainter::GetColour(Mem::Region** preg, size_t base, size_t end) const
{
	COLORREF c = RGB(0, 0, 0);

	Mem::Region*& reg = *preg;

	if (reg != 0)
	{
		while (reg->base != ((size_t)-1) && (reg->base + reg->size) < base)
		{
			++reg;
		}

		size_t tmp = 0, tmp2 = 0;
		int max_type = 0;

		Mem::Region* oreg = reg;
		while (reg->base != ((size_t)-1) && reg->base < end)
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
		/*switch (max_type)
		{
		case 0:
			c = RGB(0, 255, 0);
			break;
		case 1:
			c = RGB(255, 255, 0);
			break;
		default:
			c = RGB(255, 0, 0);
		}*/

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

	double addrmax = double(0x80000000u);

	Mem::Region* pReg = mem.head;

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	for (double d = dstart; d > ddiff; d = dnext)
	{
		dnext = d - ddiff;

		size_t base = size_t((pi - d) * addrmax / pi);
		size_t end = size_t((pi - dnext) * addrmax / pi);

		COLORREF c = GetColour(&pReg, base, end);//RGB((rand() * 256) / RAND_MAX
					//, (rand() * 256) / RAND_MAX, (rand() * 256) / RAND_MAX);
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

void MMPainter::Update()
{
	mem.Populate(procid);
}

MMPainter::Mem::Mem()
: head(0)
{
}

MMPainter::Mem::~Mem()
{
	delete[] head;
}

void MMPainter::Mem::Populate(int pid)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	Region r;
	vector<Region> rlist;

	MEMORY_BASIC_INFORMATION meminfo;
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (hProc != NULL)
	{
		for (char* p = (char*)sysinfo.lpMinimumApplicationAddress; p < (char*)sysinfo.lpMaximumApplicationAddress; p += sysinfo.dwPageSize)
		{
			VirtualQueryEx(hProc, p, &meminfo, sizeof(meminfo));

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

			rlist.push_back(r);
			if (meminfo.RegionSize > 0) p += (meminfo.RegionSize - sysinfo.dwPageSize);
		}

		CloseHandle(hProc);
	}

	size_t newsize = rlist.size() + 1;

	delete[] head;
	head = new Region[newsize];

	copy(rlist.begin(), rlist.end(), head);
	head[newsize - 1].base = (size_t)-1;
}
