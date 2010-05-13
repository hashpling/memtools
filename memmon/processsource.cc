// Copyright (c) 2008 Charles Bailey
#include "stdafx.h"
#include "processsource.h"
#include "mminfo.h"
#include "memorymap.h"

namespace MemMon
{

namespace Win
{

ProcessSource::ProcessSource( int p )
: actual_u(0.0)
, actual_k(0.0)
, ind_pos(0.0)
, ind_vel(0.0)
, last_poll(0.0)
{
	_proc = ::OpenProcess( PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, p );

	if( _proc == NULL )
		throw ConstructorFailure< ProcessSource >();
}

namespace
{

template< class T > T* StrDup( const T* );
template< class T > void StrFree( T* t ) { free(t); }

template<> char* StrDup( const char* t ) { return t ? _strdup( t ) : NULL; }
template<> wchar_t* StrDup( const wchar_t* t ) { return t ? _wcsdup( t ) : NULL; }

template< class T >
class StrDupBuffer
{
public:
	explicit StrDupBuffer( const T* t ) : _buf( StrDup( t ) ) {}
	~StrDupBuffer() { StrFree( _buf ); }

	operator T*() const { return _buf; }

private:
	T* _buf;
};

template< class T >
StrDupBuffer< T > MkDupBuffer( const T* t ) { return StrDupBuffer< T >( t ); }

class Freer
{
public:
	Freer() : _hmem( NULL ) {}

	~Freer()
	{
		::LocalFree( _hmem );
	}

	LPSTR GetHandlePtr() { return reinterpret_cast< LPSTR >( &_hmem ); }
	const char* GetString() const{ return reinterpret_cast< const char* >( _hmem ); }

private:
	HLOCAL _hmem;
};

}

ProcessSource::ProcessSource( const TCHAR* cmd, const TCHAR* args, const TCHAR* wd )
: actual_u(0.0)
, actual_k(0.0)
, ind_pos(0.0)
, ind_vel(0.0)
, last_poll(0.0)
{
	PROCESS_INFORMATION pi;

	STARTUPINFO si;
	si.cb = sizeof si;
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = 0;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;

	BOOL b = ::CreateProcess( cmd, MkDupBuffer( args ), NULL, NULL, FALSE, 0, NULL, wd, &si, &pi );

	if( b == FALSE )
	{
		Freer hloc;
		DWORD err = ::GetLastError();
		::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, err, 0, hloc.GetHandlePtr() , 0, NULL );
		throw ConstructorFailure< ProcessSource >( hloc.GetString() );
	}

	::CloseHandle( pi.hThread );
	_proc = pi.hProcess;
}

ProcessSource::~ProcessSource()
{
	::CloseHandle( _proc );
}

size_t ProcessSource::Update( MemoryMap& m )
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

	m.Stamp();

	return max_addr;
}

bool ProcessSource::Poll( double dtime, const MemMon::CPUPrefs& prefs )
{
	double k = prefs.k;
	const double delta_t = 0.1;
	double damping = prefs.damper;

	FILETIME sysidle, syskernel, sysuser;
	FILETIME proccreate, procexit, prockern, procuser;

	if( ::WaitForSingleObject( _proc, 0 ) == WAIT_OBJECT_0 )
		return false;

	GetSystemTimes(&sysidle, &syskernel, &sysuser);
	GetProcessTimes(_proc, &proccreate, &procexit, &prockern, &procuser);

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

	return true;
}

double ProcessSource::GetPos() const
{
	return ind_pos;
}

bool ProcessSource::IsPlaybackDevice() const
{
	return false;
}

}

}
