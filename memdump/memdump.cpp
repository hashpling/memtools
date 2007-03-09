// memdump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

namespace
{
	const size_t HEAP_HANDLES = 16;

	template <typename T>
	inline int get_range(T t)
	{
		int r = 0;
		while (t >= 1000)
		{
			++r;
			t /= 1024;
		}
		return r;
	}

	template <typename T>
	inline void split(T t, T& t1, T& t2, int range)
	{
		if (range > 0)
		{
			range *= 10;
			t1 = t >> range;
			if (range > 10)
			{
				t2 = (((t - (t1 << range)) >> (range - 10)) * 1000) >> 10;
			}
			else
			{
				t2 = ((t - (t1 << range)) * 1000) >> range;
			}
		}
		else
		{
			t1 = t;
			t2 = 0;
		}
	}

	char symbols[] = { ' ', 'k', 'M', 'G', 'T' };

	template <typename T>
	void format(_TCHAR* buffer, size_t s, T t, int r)
	{
		T t1, t2;
		if (r > 4) r = 4;

		switch (r)
		{
		case 0:
			_stprintf(buffer, _T("%u bytes"), t);
			break;

		default:
			split(t, t1, t2, r);
			if (t1 < 10)
			{
				T tmp = (t2 + 5) / 10;
				while (tmp >= 100) { tmp -= 100; ++t1; }
				_stprintf(buffer, _T("%u.%02u%c"), t1, tmp, symbols[r]);
			}
			else if (t1 < 100)
			{
				T tmp = (t2 + 50) / 100;
				while (tmp >= 10) { tmp -= 10; ++t1; }
				_stprintf(buffer, _T("%u.%01u%c"), t1, tmp, symbols[r]);
			}
			else
			{
				T tmp = (t2 + 500) / 1000;
				while (tmp >= 1) { tmp -= 1; ++t1; }
				_stprintf(buffer, _T("%u%c"), t1, symbols[r]);
			}
			break;
		}
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE heaps[HEAP_HANDLES];
	PHANDLE hps = heaps;
	DWORD heapcount = GetProcessHeaps(HEAP_HANDLES, hps);

	if (heapcount > HEAP_HANDLES)
	{
		hps = new HANDLE[heapcount];
		GetProcessHeaps(heapcount, hps);
	}

	PROCESS_HEAP_ENTRY phe;
	_TCHAR buf[20];

	for (DWORD d = 0; d < heapcount; ++d)
	{
		phe.lpData = NULL;

		while (HeapWalk(hps[d], &phe))
		{
			if (phe.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
			{
				format(buf, 20, phe.cbData, get_range(phe.cbData));
				_tprintf( _T("Heap entry is an uncommitted range of size %s\n"), buf );
			}

			if (phe.wFlags & PROCESS_HEAP_REGION)
			{
				_tprintf( _T("Heap entry is a region\n") );
				format(buf, 20, phe.cbData, get_range(phe.cbData));
				_tprintf( _T("Size:             %s\n"), buf );
				format(buf, 20, phe.cbOverhead, get_range(phe.cbOverhead));
				_tprintf( _T("Overhead:         %s\n"), buf );
				format(buf, 20, phe.Region.dwCommittedSize, get_range(phe.Region.dwCommittedSize));
				_tprintf( _T("Committed size:   %s\n"), buf );
				format(buf, 20, phe.Region.dwUnCommittedSize, get_range(phe.Region.dwUnCommittedSize));
				_tprintf( _T("Uncommitted size: %s\n"), buf );
			}
			else if (!(phe.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE))
			{
				format(buf, 20, phe.cbData, get_range(phe.cbData));
				_tprintf( _T("Heap entry is NOT a region of size %s\n"), buf );
			}
		}
	}

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	struct FreeInfo
	{
		FreeInfo() : p(0), s(0) {}
		void *p;
		size_t s;
	};

	FreeInfo fi[20];

	MEMORY_BASIC_INFORMATION meminfo;
	HANDLE hThisProc = GetCurrentProcess();
	for (char* p = (char*)sysinfo.lpMinimumApplicationAddress; p < (char*)sysinfo.lpMaximumApplicationAddress; p += sysinfo.dwPageSize)
	{
		VirtualQuery(p, &meminfo, sizeof(meminfo));

#ifdef WIN64
		_tprintf( _T("%016p %08x %016Ix\n"), meminfo.BaseAddress, meminfo.State, meminfo.RegionSize );
#else
		_tprintf( _T("%08p %08x %08x\n"), meminfo.BaseAddress, meminfo.State, meminfo.RegionSize );
#endif
		if (meminfo.State != MEM_FREE)
		{
			if (meminfo.Type == MEM_IMAGE || meminfo.Type == MEM_MAPPED)
			{
				_TCHAR fname[2048];
				if (GetMappedFileName(hThisProc, p, fname, sizeof(fname)))
				{
					_putts(fname);
				}
			}
		}
		else
		{
			if (fi[19].s < meminfo.RegionSize)
			{
				int j;

				for(j = 18; j >= 0; --j)
				{
					if (fi[j].s >= meminfo.RegionSize) break;
					fi[j+1].s = fi[j].s;
					fi[j+1].p = fi[j].p;
				}

				fi[j+1].s = meminfo.RegionSize;
				fi[j+1].p = p;
			}

			/*size_t minsize = (size_t)-1;
			for (size_t i = 0; i < 20; ++i)
			{
				if (fi[i].s < minsize) minsize = fi[i].s;
			}
			if (meminfo.RegionSize > minsize)
			{
				for (size_t i = 0; i < 20; ++i)
				{
					if (fi[i].s == minsize)
					{
						fi[i].p = p;
						fi[i].s = meminfo.RegionSize;
						break;
					}
				}
			}*/
		}

		if (meminfo.RegionSize > 0) p += (meminfo.RegionSize - sysinfo.dwPageSize);
	}

	if (hps != heaps) delete[] hps;

	for (size_t i = 0; i < 20; ++i)
	{
		if (fi[i].s > 0)
		{
			format(buf, 20, fi[i].s, get_range(fi[i].s));
#ifdef WIN64
			_tprintf( _T("Free block at %016p of size %s\n"), fi[i].p, buf );
#else
			_tprintf( _T("Free block at %08p of size %s\n"), fi[i].p, buf );
#endif
		}
	}
	return 0;
}

