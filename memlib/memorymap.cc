// Copyright (c) 2007,2008 Charles Bailey

#include "memorymap.h"
#include "mmintio.h"
#include <streambuf>
#include <sstream>

using std::streambuf;
using std::stringbuf;
using std::ios_base;

namespace MemMon
{

template< class StreamBuf >
void MemoryMap::Write( StreamBuf* sb ) const
{
	sb->sputn( "V1", 3 );
	sb->sputc( sizeof( size_t ) );

	size_t m = -1;

	for (RegionList::const_iterator i = _blocklist.begin(); i != _blocklist.end(); ++i)
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

	sb->sputc( '\xf0' );
}

template< class StreamBuf >
void MemoryMap::Read( StreamBuf* sb )
{
	typedef typename StreamBuf::traits_type traits_type;
	stringbuf s;
	typename traits_type::int_type t;
	while( (t = sb->sbumpc()) != 0 && t != traits_type::eof() )
		s.sputc( t );

	if (s.str() != "V1")
		throw ios_base::failure("This file is not a valid Address Space Monitor dump.");

	size_t sz = sb->sbumpc();

	if( sz > sizeof(size_t))
		throw ios_base::failure("This file is not compatible with this version of Address Space Monitor as it was saved by a version compiled for a different architecture.");

	PartialClear();

	size_t m = (size_t)-1;
	while( (t = sb->sbumpc()) != 0xf0 && t != traits_type::eof() )
	{
		Region r;
		r.type = static_cast< Region::Type >( t & 0xf );

		if( (t & 0x40) != 0 )
			MyIntGet( sb, r.base, sz );
		else
			r.base = m;

		MyIntGet( sb, r.size, sz );

		m = r.base + r.size;

		AddBlock( r );
	}
}

template void MemoryMap::Read(streambuf* sb);
template void MemoryMap::Write(streambuf* sb) const;

void MemoryMap::PartialClear()
{
	_total_free = 0;
	_total_reserve = 0;
	_total_commit = 0;

	for( FreeList::iterator i = _freelist.begin(); i != _freelist.end(); ++i )
	{
		i->size = 0;
	}
}

void MemoryMap::Clear( size_t freecount )
{
	_freelist.resize( freecount );
	_blocklist.clear();

	PartialClear();
}

void MemoryMap::UpdateFreeList( const Region& r, const Region* modified )
{
	switch( r.type )
	{
	case Region::free:
		_total_free += r.size;
		break;

	case Region::reserved:
		_total_reserve += r.size;
		break;

	default:
		_total_commit += r.size;
		break;
	}

	if( r.type != Region::free || _freelist.empty() )
		return;

	const size_t wraparound = static_cast< size_t >( -1 );

	size_t j;

	if( modified == NULL )
	{
		if( _freelist.back().size >= r.size )
			return;

		modified = &r;
		j = _freelist.size() - 1U;
	}
	else
	{
		if( _freelist.back().size >= modified->size )
			return;

		size_t oldsize = modified->size - r.size;

		bool moveold = false;

		for(j = _freelist.size() - 1U; j != wraparound; --j)
		{
			if( _freelist[j].size > oldsize ) break;
			if( _freelist[j].base == modified->base )
			{
				moveold = true;
				_freelist[j].size = modified->size;
				break;
			}
		}

		if( !moveold )
			j = _freelist.size() - 1U;
	}

	while( --j != wraparound )
	{
		if (_freelist[j].size >= modified->size) break;
		_freelist[j+1].size = _freelist[j].size;
		_freelist[j+1].base = _freelist[j].base;
	}

	_freelist[j+1].size = modified->size;
	_freelist[j+1].base = modified->base;
}

void MemoryMap::AddBlock( const Region& r )
{
	if( !_blocklist.empty()
		&& _blocklist.back().type == r.type
		&& _blocklist.back().base + _blocklist.back().size == r.base )
	{
		_blocklist.back().size += r.size;
		UpdateFreeList( r, &_blocklist.back() );
	}
	else
	{
		_blocklist.push_back( r );
		UpdateFreeList( r, NULL );
	}
}

void MemoryMap::RecalcFreeList()
{
	PartialClear();
	for( RegionList::iterator i = _blocklist.begin(); i != _blocklist.end(); ++i )
	{
		UpdateFreeList( *i, NULL );
	}
}

void MemoryMap::Swap( MemoryMap& other )
{
	std::swap( _blocklist, other._blocklist );
	std::swap( _freelist, other._freelist );
	std::swap( _total_free, other._total_free );
	std::swap( _total_commit, other._total_commit );
	std::swap( _total_reserve, other._total_reserve );
}

}
