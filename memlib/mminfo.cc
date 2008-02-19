#include "mminfo.h"

#ifdef _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
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
}

#endif

}
