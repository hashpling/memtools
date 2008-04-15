#include "playbacksource.h"
#include "mminfo.h"
#include "memorydiff.h"

using std::ios_base;

namespace MemMon
{

PlaybackSource::PlaybackSource( const char *fname )
{
	_buf.open( fname, ios_base::in | ios_base::binary );
	if( !_buf.is_open() )
		throw MemMon::ConstructorFailure< PlaybackSource >( "Failed to open recording" );

	std::streambuf* pbuf = &_buf;
	_mem.Read( pbuf );
}

PlaybackSource::~PlaybackSource()
{
}

size_t PlaybackSource::Update( MemMon::MemoryMap& m )
{
	MemoryDiff md;
	std::streambuf* pbuf = &_buf;
	md.Read( pbuf );
	md.Apply( _mem );
	m = _mem;
	if( !_mem.GetBlockList().empty() )
		return _mem.GetBlockList().back().base + _mem.GetBlockList().back().size;
	return 0;
}

bool PlaybackSource::Poll( double dtime, const MemMon::CPUPrefs& prefs )
{
	typedef std::filebuf::traits_type traits_type;
	return !traits_type::eq_int_type( _buf.sgetc(), traits_type::eof() );
}

double PlaybackSource::GetPos() const
{
	return 0.0;
}

}
