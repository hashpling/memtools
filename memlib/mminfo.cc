#include "mminfo.h"
#include "mmintio.h"
#include <ctime>

#ifdef _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

namespace MemMon
{

#ifdef _WIN32

Timestamp Timestamp::now()
{
	_timeb tb;
	_ftime( &tb );

	return Timestamp( mmint64( tb.time ) * 1000 + tb.millitm );
}

#else

Timestamp Timestamp::now()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday( &tv, &tz );
	return Timestamp( mmint64( tv.tv_sec ) * 1000 + ( tv.tv_usec / 1000 ) );
}

#endif

namespace
{
	struct BadTimestamp {};
}

Timestamp::Timestamp( const char* tmstring )
	: _msec( 0 )
{
	char* p = const_cast< char* >( tmstring );
	struct tm t;

	t.tm_year = std::strtoul( p, &p, 10 ) - 1900;

	if( p != tmstring + 4 || *p++ != '-' )
		throw BadTimestamp();

	t.tm_mon = std::strtoul( p, &p, 10 ) - 1;

	if( p != tmstring + 7 || *p++ != '-' )
		throw BadTimestamp();

	t.tm_mday = std::strtoul( p, &p, 10 );

	if( p != tmstring + 10 || *p++ != 'T' )
		throw BadTimestamp();

	t.tm_hour = std::strtoul( p, &p, 10 );

	if( p != tmstring + 13 || *p++ != ':' )
		throw BadTimestamp();

	t.tm_min = std::strtoul( p, &p, 10 );

	if( p != tmstring + 16 || *p++ != ':' )
		throw BadTimestamp();

	t.tm_sec = std::strtoul( p, &p, 10 );

	if( p != tmstring + 19 )
		throw BadTimestamp();

	if( *p++ == '.' )
	{
		_msec = strtoul( p, &p, 10 );

		ptrdiff_t nplaces = p - tmstring - 20;

		switch( nplaces )
		{
		case 0:
			break;
		case 1:
			_msec *= 100;
			break;
		case 2:
			_msec *= 10;
			break;
		case 3:
			break;
		case 4:
			_msec /= 10;
			break;
		case 5:
			_msec /= 100;
			break;
		case 6:
			_msec /= 1000;
			break;
		case 7:
			_msec /= 10000;
			break;
		default:
			throw BadTimestamp();
		}

	}

	t.tm_isdst = 0;

	time_t u = std::mktime( &t );

	if( u == (time_t)-1 )
		throw BadTimestamp();

	_msec += mmint64( u ) * 1000;
}

std::string Timestamp::GetAsString() const
{
	if( _msec == 0 )
		return std::string();

	const time_t sec = _msec / 1000;

	struct tm* t = localtime( &sec );
	char buffer[25];

	sprintf( buffer, "%04d-%02d-%02dT%02d:%02d:%02d.%03d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, int(_msec % 1000) );

	return buffer;
}

template< class StreamBuf >
void Timestamp::Write( StreamBuf* sb ) const
{
	IntPut( sb, _msec );
}

template< class StreamBuf >
void Timestamp::Read( StreamBuf* sb )
{
	IntGet( sb, _msec );
}

template void Timestamp::Write( std::streambuf* sb ) const;
template void Timestamp::Read( std::streambuf* sb );

template< class StreamBuf >
void TimeInterval::Write( StreamBuf* sb ) const
{
	if( _msec >= 0 && _msec < 0x8000 )
	{
		// Use just two bytes for up to 32 and a bit positive seconds
		IntPut( sb, _msec, 2 );
	}
	else if( _msec < 0 )
	{
		// Technically UB if -_msec turns out negative again
		IntPut( sb, 0xc000 | -_msec % 0x4000, 2 );
		IntPut( sb, -_msec / 0x4000, 4 );
	}
	else
	{
		IntPut( sb, 0x8000 | _msec % 0x4000, 2 );
		IntPut( sb, _msec / 0x4000, 4 );
	}
}

template< class StreamBuf >
void TimeInterval::Read( StreamBuf* sb )
{
	IntGet( sb, _msec, 2 );
	if( _msec & 0x8000 )
	{
		mmint64 tmp;
		IntGet( sb, tmp, 4 );

		if( _msec & 0x4000 )
			_msec = -(tmp * 0x4000 + (_msec & 0x3fff));
		else
			_msec = tmp * 0x4000 + (_msec & 0x3fff);
	}
}

template void TimeInterval::Write( std::streambuf* sb ) const;
template void TimeInterval::Read( std::streambuf* sb );

}
