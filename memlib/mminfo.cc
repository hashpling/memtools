#include "mminfo.h"
#include "mmintio.h"

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

	return mmint64( tb.time ) * 1000 + tb.millitm;
}

#else

Timestamp Timestamp::now()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday( &tv, &tz );
	return mmint64( tv.tv_sec ) * 1000 + ( tv.tv_usec / 1000 );
}

#endif

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

}
