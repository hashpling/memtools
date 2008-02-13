// Copyright (c) 2008 Charles Bailey

#include "mminfo.h"
#include <sstream>

using std::vector;
using std::basic_streambuf;
using std::basic_ostream;
using std::basic_istream;
using std::streambuf;
using std::stringbuf;
using std::ios_base;

namespace MemMon
{

namespace
{

template< class StreamBuf, class intT >
void MyIntPut( StreamBuf* sb, intT toput, size_t width = sizeof(intT) )
{
	for (unsigned i = 0; i < width - 1; ++i)
	{
		sb->sputc( toput & 0xff );
		toput >>= 8;
	}
	sb->sputc( toput & 0xff );
}

template< class StreamBuf, class intT >
void MyIntGet( StreamBuf* sb, intT& toget, size_t width = sizeof(intT) )
{
	toget = 0;
	for (unsigned i = 0; i < width; ++i)
	{
		toget |= intT(sb->sbumpc()) << (i * 8);
	}
}

}

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

	Clear();

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
	case 0:
		_total_free += r.size;
		break;

	case 1:
		_total_reserve += r.size;
		break;

	default:
		_total_commit += r.size;
		break;
	}

	if( _freelist.empty() )
		return;

	if( r.type == 0 )
	{
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

MemoryDiff::MemoryDiff( const MemoryMap& before, const MemoryMap& after )
{
	RegionList::const_iterator bit = before.GetBlockList().begin();
	RegionList::const_iterator ait = after.GetBlockList().begin();
	const RegionList::const_iterator bend = before.GetBlockList().end();
	const RegionList::const_iterator aend = after.GetBlockList().end();

	while( ait != aend || bit != bend )
	{
		if( bit == bend )
		{
			while( ait != aend )
				AppendAddition( *ait++ );
			break;
		}

		if( ait == aend )
		{
			while( bit != bend )
				AppendRemoval( *bit++ );
			break;
		}

		// If we get here we have two non-ends
		if( ait->base < bit->base )
			AppendAddition( *ait++ );
		else if( ait->base > bit->base )
			AppendRemoval( *bit++ );
		else
		{
			if( ait->size < bit->size )
			{
				bool bOriginalPreserved = false;

				do
				{
					if( ait->type == bit->type )
						bOriginalPreserved = true;
					else
						AppendAddition( *ait );
				} while( ++ait != aend && ait->base < bit->base + bit->size );

				if( !bOriginalPreserved )
				{
					_changes.back().first = change;
					_changes.back().second.first = *bit;
				}

				++bit;
			}
			else if ( ait->size > bit->size )
			{
				RegionList toremove;

				if( ait->type != bit->type )
					toremove.push_back( *bit );

				while( ++bit != bend && bit->base <= ait->base + ait->size )
				{
					if( bit->type != ait->type )
					{
						toremove.push_back( *bit );
					}
					else if( !toremove.empty() )
					{
						size_t base = toremove.front().base;
						for( RegionList::iterator i = toremove.begin(); i != toremove.end(); ++i )
							AppendRemoval( Region( base, i->base + i->size - base, i->type ) );
						toremove.clear();
					}
				}

				if( !toremove.empty() )
				{
					size_t base = toremove.front().base;
//					const RegionList::iterator lastbutone =  - 1;

					for( RegionList::iterator i = toremove.begin(); i != toremove.end(); ++i )
						AppendRemoval( Region( base, i->base + i->size - base, i->type ) );

					if( !_changes.empty() )
					{
						_changes.back().first = change;
						_changes.back().second.second = Region( base, ait->base + ait->size - base, ait->type );
					}
				}
				/*bool bNewPreserved = false;

				do
				{
					if( ait->type == bit->type )
						bNewPreserved = true;
					else
						AppendRemoval( *bit );
				} while( ++bit != bend && bit->base < ait->base + ait->size );

				if( !bNewPreserved )
				{
					_changes.back().first = change;
					_changes.back().second.second = *ait;
				}*/

				++ait;
			}
			else if ( ait->type != bit->type )
			{
				AppendChange( *bit++, *ait++ );
			}
			else
			{
				++ait;
				++bit;
			}
		}
	}
}

namespace
{

class PatchFailed
{
public:
	enum Reason
	{
		  AdditionOverwrites
		, NoMatchForChange
		, AdditionBeyondLast
	};

	PatchFailed( Reason r ) : _reason( r ) {}

private:
	Reason _reason;
};

void DoAdd( RegionList& blocklist, RegionList::iterator& i, const Region& r )
{
	while( i != blocklist.end() && i->base + i->size <= r.base )
		++i;

	if( i == blocklist.end() )
		throw PatchFailed( PatchFailed::AdditionBeyondLast );

	if( i->base == r.base )
	{
		// Insertion at start of existing region
		if( i->size <= r.size )
			throw PatchFailed( PatchFailed::AdditionOverwrites );

		i->base += r.size;
		i->size -= r.size;

		i = blocklist.insert( i, r );
		++i;
	}
	else
	{
		Region x;
		x.size = 0;

		if( i->base + i->size > r.base + r.size )
		{
			// Existing region wholly encloses addition
			// Split original into two
			x.base = r.base + r.size;
			x.size = i->base + i->size - x.base;
			x.type = i->type;
		}

		// Shrink preceding region
		i->size = r.base - i->base;

		if( x.size != 0 )
			i = blocklist.insert( ++i, x );
		else
			++i;

		i = blocklist.insert( i, r );
		++i;

		// If new region encroaches into next region, then shrink it
		if( i != blocklist.end() && i->base < r.base + r.size )
		{
			i->size -= r.base + r.size - i->base;
			i->base = r.base + r.size;
		}
	}
}

void DoRemove( RegionList& blocklist, RegionList::iterator& i, const Region& r )
{
	while( i != blocklist.end() && i->base + i->size <= r.base )
		++i;

	i = blocklist.erase( i );

	if( i != blocklist.end() )
	{
		i->base -= r.size;
		i->size += r.size;

		if( i != blocklist.begin() )
		{
			RegionList::iterator j = i - 1;
			if( j->type == i->type && j->base + j->size == i->base )
			{
				j->size += i->size;
				i = blocklist.erase( i );
			}
		}
	}
}

void DoChange( RegionList& blocklist, RegionList::iterator& i, const Region& r )
{
	size_t change_base = r.base;

	while( i != blocklist.end() && i->base + i->size <= change_base )
		++i;

	if( i == blocklist.end() )
		throw PatchFailed( PatchFailed::NoMatchForChange );

	*i = r;
}

}

void MemoryDiff::Apply( MemoryMap& target ) const
{
	RegionList& blocklist = target.GetBlockListRef();
	RegionList::iterator i = blocklist.begin();

	for( Changes::const_iterator cit = _changes.begin(); cit != _changes.end(); ++cit )
	{
		switch( cit->first )
		{
		case change:
			DoChange( blocklist, i, cit->second.second );
			break;
		case removal:
			DoRemove( blocklist, i, cit->second.first );
			break;
		case addition:
			DoAdd( blocklist, i, cit->second.second );
			break;
		}
	}

	target.RecalcFreeList();
}

template< class StreamBuf >
void MemoryDiff::Write( StreamBuf* sb ) const
{
	sb->sputn( "md", 3 );
	sb->sputc( sizeof( size_t ) );

	for( Changes::const_iterator i = _changes.begin(); i != _changes.end(); ++i )
	{
		switch( i->first )
		{
		case addition:
			sb->sputc( '\0' );
			MyIntPut( sb, i->second.second.base );
			MyIntPut( sb, i->second.second.size );
			sb->sputc( i->second.second.type );
			break;

		case removal:
			sb->sputc( '\01' );
			MyIntPut( sb, i->second.first.base );
			MyIntPut( sb, i->second.first.size );
			sb->sputc( i->second.first.type );
			break;

		case change:
			sb->sputc( '\02' );
			MyIntPut( sb, i->second.first.base );
			MyIntPut( sb, i->second.first.size );
			sb->sputc( i->second.first.type );
			MyIntPut( sb, i->second.second.base );
			MyIntPut( sb, i->second.second.size );
			sb->sputc( i->second.second.type );
			break;
		}
	}
	sb->sputc( '\xf0' );
}

template< class StreamBuf >
void MemoryDiff::Read( StreamBuf* sb )
{
	typedef typename StreamBuf::traits_type traits_type;
	Change c;

	char b[4];

	if( sb->sgetn( b, 4 ) != 4 )
		throw ReadError( "MemoryDiff signature is incomplete" );

	if( memcmp( b, "md", 3) != 0 )
		throw ReadError( "MemoryDiff signature is incorrect" );

	size_t sz = b[3];

	typename traits_type::int_type j;

	while( (j = sb->sbumpc()) != traits_type::eof() && j != 0xf0 )
	{
		switch( j )
		{
		case 0:
			c.first = addition;
			MyIntGet( sb, c.second.second.base, sz );
			MyIntGet( sb, c.second.second.size );
			c.second.second.type = static_cast< Region::Type >( sb->sbumpc() );
			break;

		case 1:
			c.first = removal;
			MyIntGet( sb, c.second.first.base, sz );
			MyIntGet( sb, c.second.first.size, sz );
			c.second.first.type = static_cast< Region::Type >( sb->sbumpc() );
			break;

		case 2:
			c.first = change;
			MyIntGet( sb, c.second.first.base, sz );
			MyIntGet( sb, c.second.first.size, sz );
			c.second.first.type = static_cast< Region::Type >( sb->sbumpc() );
			MyIntGet( sb, c.second.second.base, sz );
			MyIntGet( sb, c.second.second.size, sz );
			c.second.second.type = static_cast< Region::Type >( sb->sbumpc() );
			break;
		}

		_changes.push_back( c );
	}
}

template void MemoryDiff::Write( std::streambuf* ) const;
template void MemoryDiff::Read( std::streambuf* );

}
