#include "stdafx.h"
#include "mminfo.h"
#include <sstream>

using std::vector;
using std::basic_streambuf;
using std::basic_ostream;
using std::basic_istream;
using std::streambuf;
using std::stringbuf;
using std::ios_base;

namespace MMInfo
{

size_t MemoryMap::Populate(HANDLE hProc)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	size_t max_addr = (size_t)sysinfo.lpMaximumApplicationAddress;

	Region r;

	freelist.resize(50);
	blocklist.clear();

	total_free = 0;
	total_reserve = 0;
	total_commit = 0;

	for (vector<FreeRegion>::iterator i = freelist.begin();
									i != freelist.end(); ++i)
	{
		i->size = 0;
	}

	MEMORY_BASIC_INFORMATION meminfo;

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
			total_free += r.size;
			break;
		case MEM_RESERVE:
			r.type = 1;
			total_reserve += r.size;
			break;
		case MEM_COMMIT:
		default:
			r.type = 2;
			total_commit += r.size;
			break;
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

	max_addr = (size_t)meminfo.BaseAddress + meminfo.RegionSize;

	return max_addr;
}

template<typename charT, typename traits, typename intT>
void MyIntPut( basic_streambuf<charT, traits>* sb, intT toput )
{
	for (int i = 0; i < sizeof(intT) - 1; ++i)
	{
		sb->sputc(charT(toput & 0xff));
		toput >>= 8;
	}
	sb->sputc(charT(toput & 0xff));
}

template<typename charT, typename traits, typename intT>
void MyIntGet( basic_streambuf<charT, traits>* sb, intT& toget )
{
	toget = 0;
	for (int i = 0; i < sizeof(intT); ++i)
	{
		toget |= intT(sb->sbumpc()) << (i * 8);
	}
}

template<typename charT, typename traits>
void MemoryMap::Write(basic_streambuf<charT, traits>* sb) const
{
	sb->sputn("V1", 3);
	sb->sputc(charT(sizeof(size_t)));

	size_t m = (size_t)-1;

	for (vector<Region>::const_iterator i = blocklist.begin(); i != blocklist.end(); ++i)
	{
		if (m != i->base)
		{
			sb->sputc(i->type | 0x40);
			MyIntPut(sb, i->base);
		}
		else
		{
			sb->sputc(i->type);
		}

		MyIntPut(sb, i->size);

		m = i->base + i->size;
	}

	sb->sputc('\xf0');
}

template<typename charT, typename traits>
void MemoryMap::Read(basic_streambuf<charT, traits>* sb)
{
	stringbuf s;
	charT t;
	while ((t = sb->sbumpc()) != '\0' && t != traits::eof())
	{
		s.sputc(t);
	}

	if (s.str() != "V1")
	{
		throw ios_base::failure("This file is not a valid Address Space Monitor dump.");
	}

	if (sb->sbumpc() != sizeof(size_t))
	{
		throw ios_base::failure("This file is not compatible with this version of Address Space Monitor as it was saved by a version compiled for a different architecture.");
	}

	total_free = 0;
	total_reserve = 0;
	total_commit = 0;

	blocklist.clear();
	freelist.resize(50);
	for (vector<FreeRegion>::iterator i = freelist.begin();
									i != freelist.end(); ++i)
	{
		i->size = 0;
	}

	size_t m = (size_t)-1;
	while ((t = sb->sbumpc()) != '\xf0' && t != traits::eof())
	{
		Region r;
		r.type = t & 0xf;

		if ((t & 0x40) != 0)
		{
			MyIntGet(sb, r.base);
		}
		else
		{
			r.base = m;
		}

		MyIntGet(sb, r.size);

		switch(r.type)
		{
		case 0:
			total_free += r.size;
			break;
		case 1:
			total_reserve += r.size;
			break;
		case 2:
			total_commit += r.size;
			break;
		}

		m = r.base + r.size;

		blocklist.push_back(r);

		if (r.type == 0 && freelist.back().size < r.size)
		{
			int j;

			for(j = int(freelist.size()) - 2; j >= 0; --j)
			{
				if (freelist[j].size >= r.size) break;
				freelist[j+1].size = freelist[j].size;
				freelist[j+1].base = freelist[j].base;
			}

			freelist[j+1].size = r.size;
			freelist[j+1].base = r.base;
		}

	}
}

template void MemoryMap::Read(streambuf* sb);
template void MemoryMap::Write(streambuf* sb) const;

}