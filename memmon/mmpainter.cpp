// Copyright (c) 2007,2008 Charles Bailey

#include "stdafx.h"
#include "mmpainter.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "hrfmt.h"
#include "mmprefs.h"
#include "shlobj.h"
#include <fstream>
#include <sstream>
#include <cassert>

using std::cos;
using std::sin;
using std::vector;
using std::copy;
using std::ofstream;
using std::ifstream;
using std::ios_base;
using std::istream;
using std::ostream;
using std::basic_istream;
using std::basic_ostream;
using std::basic_streambuf;
using std::stringbuf;
using std::exception;

using MemMon::MemoryMap;
using MemMon::FreeRegion;
using MemMon::Region;

MMPainter::MMPainter(int r, MMPrefs* p)
: hMemDC(NULL), hBmp(NULL), hOldBmp(NULL)
, radius(r), dradius(r), width(r * 3 /  5), maxaddr(0x1000000u)
, next_update(0.0), pPrefs(p)
{
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	hWBrush = CreateSolidBrush(RGB(255, 255, 255));
	hPen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));

	rsize.left = 0;
	rsize.top = 0;
	rsize.right = radius << 1;
	rsize.bottom = radius + 180 + 16;

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	maxaddr = (size_t)sysinfo.lpMaximumApplicationAddress;
	processor_count = (double)sysinfo.dwNumberOfProcessors;
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
		hBmp = CreateCompatibleBitmap(hdc, rsize.right, rsize.bottom);
		hOldBmp = SelectObject(hMemDC, hBmp);

		SetBkColor(hMemDC, RGB(255, 255, 255));
	}

	int x, y, w, h;
	x = max(ps->rcPaint.left, 0);
	y = max(ps->rcPaint.top, 0);
	w = min(ps->rcPaint.right - x, (rsize.right) - x);
	h = min(ps->rcPaint.bottom - y, (rsize.bottom) - y);
	BitBlt(hdc, x, y, w, h, hMemDC, x, y, SRCCOPY);
}

void MMPainter::MemPaint(HDC hdc) const
{
	FillRect(hdc, &rsize, hWBrush);
	DisplayGauge(hdc, false);
	DisplayBlobs(hdc);
	DisplayTotals(hdc, 280);
}


COLORREF MMPainter::GetColour(vector<Region>::const_iterator& reg
							, vector<Region>::const_iterator rend
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

		vector<Region>::const_iterator oreg = reg;
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

void MMPainter::DisplayGauge(HDC hdc, bool bQuick) const
{
	double pi = acos(-1.0);

	if (!bQuick)
	{
		double dstart = pi;
		double ddiff = dstart / (dradius*2.0);
		double dnext;

		double addrmax = double(maxaddr);

		vector<Region>::const_iterator pReg = mem.GetBlockList().begin();

		HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
		HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
		for (double d = dstart; d > ddiff; d = dnext)
		{
			dnext = d - ddiff;

			size_t base = size_t((pi - d) * addrmax / pi);
			size_t end = size_t((pi - dnext) * addrmax / pi);

			COLORREF c = GetColour(pReg, mem.GetBlockList().end(), base, end);
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
	//
	HGDIOBJ hOldPen = SelectObject(hdc, hPen);
	HGDIOBJ hOldBrush = SelectObject(hdc, hWBrush);
	int ww = (radius << 1) - width;
	Pie(hdc, width, width, ww, ww, ww, radius, width, radius + 1);
	SelectObject(hdc, hOldPen);
	Arc(hdc, width, width, ww, ww, ww, radius, width, radius + 1);
	Arc(hdc, 0, 0, radius << 1, radius << 1, radius << 1, radius, 0, radius + 1);

	if( _source.get() )
	{
		double dpos = _source->GetPos() * pi;

		if (pPrefs != NULL && pPrefs->GetCPUPrefs().use_cpu_count)
		{
			dpos /= processor_count;
		}

		SelectObject(hdc, GetStockObject(DC_BRUSH));
		SelectObject(hdc, GetStockObject(DC_PEN));
		SetDCBrushColor(hdc, RGB(127, 0, 0));
		SetDCPenColor(hdc, RGB(127, 0, 0));

		if (dpos < 0.0) dpos = 0.0;
		else if (dpos > 2 * pi) dpos = 2 * pi;

		int x1 = radius - int(dradius * cos(dpos));
		int y1 = radius - int(dradius * sin(dpos));
		int x2 = 0;
		int y2 = radius;

		if (y1 == radius) y1 -= 1;

		Pie(hdc, width + 3, width + 3, ww - 3, ww - 3, x1, y1, x2, y2);

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
}

COLORREF MMPainter::GetBlobColour(const FreeRegion& reg) const
{
	int c = int(511.0 * double(reg.size + reg.base)/double(maxaddr));
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

	return RGB(r, g, b);
}

void MMPainter::DisplayBlobs(HDC hdc) const
{
	int currentline = 1;
	int currentpos = 0;

	HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
	HGDIOBJ hOldPen = SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, RGB(0, 0, 0));

	for (vector<FreeRegion>::const_iterator k = mem.GetFreeList().begin();
			k != mem.GetFreeList().end(); ++k)
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

		SetDCBrushColor(hdc, GetBlobColour(*k));
		Ellipse(hdc, currentpos, radius + 20 * currentline, currentpos + nwidth
											, radius + 20 * (currentline + 1));

		currentpos += nwidth + 1;

	}

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
}

void MMPainter::DisplayTotals(HDC hdc, int offset) const
{
	COLORREF colcomm = RGB(170, 0, 0);
	COLORREF colresv = RGB(170, 170, 0);
	COLORREF colfree = RGB(0, 170, 0);

	COLORREF txtcol = GetTextColor(hdc);

	_TCHAR buf[100];
	RECT rtmp;

	TEXTMETRIC txtmet;
	GetTextMetrics(hdc, &txtmet);

	rtmp.left = 0;
	rtmp.top = offset;
	rtmp.right = rsize.right / 4;
	rtmp.bottom = rtmp.top + txtmet.tmHeight;

	HRFormat::hr_format(buf, 100, mem.GetCommitTotal());
	SetTextColor(hdc, colcomm);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.GetReserveTotal());
	SetTextColor(hdc, colresv);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	rtmp.left = rtmp.right;
	rtmp.right += rsize.right / 4;

	HRFormat::hr_format(buf, 100, mem.GetFreeTotal());
	SetTextColor(hdc, colfree);
	DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);

	if (!mem.GetFreeList().empty())
	{
		const FreeRegion& fr = mem.GetFreeList().front();

		rtmp.left = rtmp.right;
		rtmp.right += rsize.right / 4;

		COLORREF bcol = GetBlobColour(fr);
		COLORREF bcol2 = RGB(GetRValue(bcol)*2/3, GetGValue(bcol)*2/3
												, GetBValue(bcol)*2/3);
		HRFormat::hr_format(buf, 100, fr.size);
		SetTextColor(hdc, bcol2);
		DrawText(hdc, buf, -1, &rtmp, DT_NOPREFIX);
	}

	SetTextColor(hdc, txtcol);
}

namespace
{
	template< class T >
	struct ConstructorFailure {};
}

MMPainter::ProcessSource::ProcessSource( int p )
: actual_u(0.0)
, actual_k(0.0)
, ind_pos(0.0)
, ind_vel(0.0)
, last_poll(0.0)
{
	_proc = ::OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, p );

	if( _proc == NULL )
		throw ConstructorFailure< ProcessSource >();
}

MMPainter::ProcessSource::~ProcessSource()
{
	::CloseHandle( _proc );
}

void MMPainter::SetProcessId(int p)
{
	try
	{
		_source.reset( new ProcessSource( p ) );
	}
	catch( ConstructorFailure< ProcessSource >& )
	{
	}
}

size_t MMPainter::ProcessSource::Update( MemoryMap& m )
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	size_t max_addr = (size_t)sysinfo.lpMaximumApplicationAddress;

	Region r;
	m.Clear();

	MEMORY_BASIC_INFORMATION meminfo;

	for (char* p = (char*)sysinfo.lpMinimumApplicationAddress;
		p < (char*)sysinfo.lpMaximumApplicationAddress;
		p += sysinfo.dwPageSize)
	{
		VirtualQueryEx( _proc, p, &meminfo, sizeof(meminfo) );

		if (p != meminfo.BaseAddress) break;

		r.base = (size_t)meminfo.BaseAddress;
		r.size = meminfo.RegionSize;
		switch (meminfo.State)
		{
		case MEM_FREE:
			r.type = Region::free;
			break;
		case MEM_RESERVE:
			r.type = Region::reserved;
			break;
		case MEM_COMMIT:
		default:
			r.type = Region::committed;
			break;
		}

		m.AddBlock( r );

		if (meminfo.RegionSize > 0)
		{
			p += (meminfo.RegionSize - sysinfo.dwPageSize);
		}
	}

	max_addr = (size_t)meminfo.BaseAddress + meminfo.RegionSize;

	return max_addr;
}

void MMPainter::Update()
{
	if (_source.get() != NULL)
	{
		double ctime = _source->Poll( pPrefs->GetCPUPrefs() );

		if (ctime > next_update)
		{
			maxaddr = _source->Update( _memprev );
			std::swap( mem, _memprev );

			if( _recorder.get() )
			{
				_recorder->Record( _memprev, mem );
			}

			if (hMemDC != NULL)
			{
				MemPaint(hMemDC);
			}
			next_update = ctime + 0.999;
		}
		else
		{
			if (hMemDC != NULL)
			{
				DisplayGauge(hMemDC, true);
			}
		}
	}
}

inline double FT2dbl(LPFILETIME lpFt)
{
	__int64 tmp = (__int64(lpFt->dwHighDateTime) << 32) + __int64(lpFt->dwLowDateTime);
	return double(tmp) / 10000000.0;
}

double MMPainter::ProcessSource::Poll( const MemMon::CPUPrefs& prefs )
{
	double k = prefs.k;
	const double delta_t = 0.1;
	double damping = prefs.damper;

	FILETIME currtime;
	FILETIME sysidle, syskernel, sysuser;
	FILETIME proccreate, procexit, prockern, procuser;

	GetSystemTimeAsFileTime(&currtime);
	GetSystemTimes(&sysidle, &syskernel, &sysuser);
	GetProcessTimes(_proc, &proccreate, &procexit, &prockern, &procuser);

	double dtime = FT2dbl(&currtime);

	double new_u = FT2dbl(&procuser);
	double new_k = FT2dbl(&prockern);

	double cpufrac = ((new_u - actual_u) + (new_k - actual_k)) / (dtime - last_poll);

	if (last_poll > 0.0)
	{
		for (double time = last_poll; time < dtime; time += delta_t)
		{
			double daccel = k * (cpufrac - ind_pos) - damping * ind_vel;
			ind_vel += daccel * delta_t;
			ind_pos += ind_vel * delta_t;
		}
	}

	actual_u = new_u;
	actual_k = new_k;
	last_poll = dtime;

	return dtime;
}

double MMPainter::ProcessSource::GetPos() const
{
	return ind_pos;
}

namespace
{

bool DoSaveDialog( HWND hwnd, char* filename )
{
	filename[0] = 0;

	OPENFILENAMEA ofn;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOTESTFILECREATE | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.pvReserved = NULL;
	ofn.dwReserved = NULL;
	ofn.FlagsEx = 0;

	return GetSaveFileNameA(&ofn) == TRUE;
}

}

void MMPainter::Snapshot(HWND hwnd) const
{
	char filename[MAX_PATH];

	if (DoSaveDialog(hwnd, filename))
	{
		ofstream ofs(filename, ios_base::out | ios_base::binary);

		try
		{
			std::ostream& o = ofs;
			o << mem;

			if (ofs.fail())
			{
				MessageBox(hwnd, _T("There was an error writing out the dump."), _T("Write Error"), MB_ICONWARNING | MB_OK);
			}
		}
		catch (exception& ex)
		{
			MessageBoxA(hwnd, ex.what(), "Write Error", MB_ICONINFORMATION | MB_OK);
		}
	}
}

void MMPainter::Read(HWND hwnd)
{
	char filename[MAX_PATH];
	filename[0] = 0;

	OPENFILENAMEA ofn;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOTESTFILECREATE | OFN_FILEMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.pvReserved = NULL;
	ofn.dwReserved = NULL;
	ofn.FlagsEx = 0;

	if (GetOpenFileNameA(&ofn))
	{
		_recorder.reset();

		ifstream ifs(filename, ios_base::in | ios_base::binary);

		try
		{
			std::istream& i = ifs;
			i >> mem;

			if (ifs.fail())
			{
				MessageBox(hwnd, _T("There was an error reading the dump."), _T("Read Error"), MB_ICONWARNING | MB_OK);
			}
			else
			{
				_source.reset();

				const Region& r = mem.GetBlockList().back();
				maxaddr = r.base + r.size;

				if (hMemDC != NULL)
				{
					MemPaint(hMemDC);
				}
			}
		}
		catch (exception& ex)
		{
			MessageBoxA(hwnd, ex.what(), "Read Error", MB_ICONINFORMATION | MB_OK);
		}
	}
}

bool MMPainter::Record(HWND hwnd)
{
	char filename[MAX_PATH];

	if (DoSaveDialog(hwnd, filename))
	{
		try
		{
			_recorder.reset( new FStreamRecorder( filename, mem ) );
			return true;
		}
		catch (...)
		{
		}
	}
	return false;
}

MMPainter::FStreamRecorder::FStreamRecorder( const char* fname, const MemoryMap& mm )
#ifdef MEMMON_DEBUG
: _fname( fname )
, _count( 0 )
#endif
{
	_buf.open( fname, std::ios_base::out | std::ios_base::binary );

	if( !_buf.is_open() )
		throw ConstructorFailure< FStreamRecorder >();

	std::streambuf* bufptr = &_buf;
	mm.Write( bufptr );
}

MMPainter::FStreamRecorder::~FStreamRecorder()
{
}

void MMPainter::FStreamRecorder::Record( const MemoryMap& l, const MemoryMap& r )
{
	MemMon::MemoryDiff d( l, r );
	std::streambuf* bufptr = &_buf;
	d.Write( bufptr );
#ifdef MEMMON_DEBUG
	std::ostringstream tmpname;
	tmpname << _fname << "_DEBUG_" << ++_count;
	ofstream fstmp( tmpname.str().c_str(), std::ios_base::out | std::ios_base::binary );
	ostream& stmp = fstmp;
	stmp << r;
#endif
}
