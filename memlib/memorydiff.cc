// Copyright (c) 2008 Charles Bailey

#include "memorydiff.h"
#include "memorymap.h"
#include "mmintio.h"

namespace MemMon
{

MemoryDiff::MemoryDiff( const MemoryMap& before, const MemoryMap& after )
	: _ts( after.GetTimestamp() )
{
	RegionList::const_iterator bit = before.GetBlockList().begin();
	RegionList::const_iterator ait = after.GetBlockList().begin();
	const RegionList::const_iterator bend = before.GetBlockList().end();
	const RegionList::const_iterator aend = after.GetBlockList().end();

	size_t commonbase = 0;

	while( ait != aend || bit != bend )
	{
		if( bit == bend )
		{
			while( ait != aend )
				_changes.push_back( Changes::value_type( new Addition( *ait++ ) ) );
			break;
		}

		if( ait == aend )
		{
			while( bit != bend )
				_changes.push_back( Changes::value_type( new Removal( (bit++)->base) ) );
			break;
		}

		size_t bbase = std::max( commonbase, bit->base );
		size_t abase = std::max( commonbase, ait->base );

		// If we get here we have two non-ends
		if( abase < bbase )
			_changes.push_back( Changes::value_type( new Addition( *ait++ ) ) );
		else if( abase > bbase )
			_changes.push_back( Changes::value_type( new Removal( (bit++)->base ) ) );
		else
		{
			size_t bsize = bit->base + bit->size - bbase;
			size_t asize = ait->base + ait->size - abase;

			if( asize < bsize )
			{
				const RegionList::const_iterator a2it = ait + 1;
				if( a2it != aend && ait->size + a2it->size > bit->size )
				{
					commonbase = bit->base + bit->size;
					_changes.push_back( Changes::value_type( new DetailChange( *ait ) ) );
					do
					{
						++ait;
					} while ( ait != aend && ait->base + ait->size <= commonbase );
					++bit;
				}
				else
				{
					if( ait->type != bit->type )
					{
						_changes.push_back( Changes::value_type( new Addition( *ait ) ) );
						commonbase = ait->base + ait->size;
						++ait;
					}
					else if( ++ait != aend )
					{
						_changes.push_back( Changes::value_type( new Addition( *ait ) ) );
						commonbase = ait->base + ait->size;
						++ait;
						while ( bit != bend && bit->base + bit->size <= commonbase )
							++bit;
					}
				}
			}
			else if ( asize > bsize )
			{
				const RegionList::const_iterator b2it = bit + 1;
				if( bit->type != ait->type || b2it != bend && bsize + b2it->size >= asize )
				{
					commonbase = ait->base + ait->size;
					_changes.push_back( Changes::value_type( new DetailChange( *ait ) ) );
					++ait;
					do
					{
						++bit;
					} while ( bit!= bend && bit->base + bit->size <= commonbase );
				}
				else
				{
					if( ++bit != bend )
					{
						_changes.push_back( Changes::value_type( new Removal( bit->base ) ) );
						commonbase = bit->base + bit->size;
						++bit;
						while ( ait != aend && ait->base + ait->size <= commonbase )
							++ait;
					}
				}
			}
			else if ( ait->type != bit->type )
			{
				commonbase = bit->base + bit->size;
				_changes.push_back( Changes::value_type( new DetailChange( *ait++ ) ) );
				++bit;
			}
			else
			{
				commonbase = bit->base + bit->size;
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

}

void MemoryDiff::Addition::Apply( RegionList& blocklist, RegionList::iterator& i ) const
{
	while( i != blocklist.end() && i->base + i->size <= _r.base )
		++i;

	if( i == blocklist.end() )
		throw PatchFailed( PatchFailed::AdditionBeyondLast );

	if( i->base == _r.base )
	{
		// Insertion at start of existing region
		if( i->size <= _r.size )
			throw PatchFailed( PatchFailed::AdditionOverwrites );

		i->base += _r.size;
		i->size -= _r.size;

		i = blocklist.insert( i, _r );
		++i;
	}
	else
	{
		Region x;
		x.size = 0;

		if( i->base + i->size > _r.base + _r.size )
		{
			// Existing region wholly encloses addition
			// Split original into two
			x.base = _r.base + _r.size;
			x.size = i->base + i->size - x.base;
			x.type = i->type;
		}

		// Shrink preceding region
		i->size = _r.base - i->base;

		if( x.size != 0 )
			i = blocklist.insert( ++i, x );
		else
			++i;

		i = blocklist.insert( i, _r );
		++i;

		// If new region encroaches into next region, then shrink it
		if( i != blocklist.end() && i->base < _r.base + _r.size )
		{
			i->size -= _r.base + _r.size - i->base;
			i->base = _r.base + _r.size;
			if( i->size == 0 )
				i = blocklist.erase( i );
		}
	}
}

void MemoryDiff::Removal::Apply( RegionList& blocklist, RegionList::iterator& i ) const
{
	while( i != blocklist.end() && i->base + i->size <= _b )
		++i;

	if( i == blocklist.end() )
		return;

	size_t s = i->size;

	i = blocklist.erase( i );

	if( i != blocklist.begin() )
	{
		const RegionList::iterator j = i - 1;
		j->size += s;

		if( i != blocklist.end() )
		{
			// Merge adjacent blocks of the same type
			if( i->type == j->type )
			{
				j->size += i->size;
				i = blocklist.erase( i );
			}
		}
		// Go back to previous block
		--i;
	}
	else if( i != blocklist.end() )
	{
		i->base -= s;
		i->size += s;

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

void MemoryDiff::DetailChange::Apply( RegionList& blocklist, RegionList::iterator& i ) const
{
	while( i != blocklist.end() && i->base + i->size <= _r.base )
		++i;

	if( i == blocklist.end() )
		throw PatchFailed( PatchFailed::NoMatchForChange );

	size_t iend = i->base + i->size;
	size_t rend = _r.base + _r.size;

	if( iend > rend )
	{
		const RegionList::iterator j = i + 1;
		if( j != blocklist.end() )
		{
			size_t diff = iend - rend;
			j->base -= diff;
			j->size += diff;
		}
	}
	else if( iend < rend )
	{
		RegionList::iterator j = i + 1;
		size_t diff = rend - iend;
		while( diff != 0 && j != blocklist.end() )
		{
			size_t reduce = std::min( diff, j->size );
			diff -= reduce;
			j->base += reduce;
			j->size -= reduce;
			if( j->size == 0 )
				j = blocklist.erase( j );
		}
	}

	i->size = _r.size;
	i->type = _r.type;
}

void MemoryDiff::Apply( MemoryMap& target ) const
{
	RegionList& blocklist = target.GetBlockListRef();
	RegionList::iterator i = blocklist.begin();

	for( Changes::const_iterator cit = _changes.begin(); cit != _changes.end(); ++cit )
	{
		(*cit)->Apply( blocklist, i );
	}

	target.RecalcFreeList();
}

template< class StreamBuf >
void DoWrite( const MemoryDiff::Change*, StreamBuf* );

template<> void DoWrite( const MemoryDiff::Change* c, std::streambuf* sb )
{
	c->Write( sb );
}

void MemoryDiff::Addition::Write( std::streambuf* sb ) const
{
	sb->sputc( '\0' );
	MyIntPut( sb, _r.base );
	MyIntPut( sb, _r.size );
	sb->sputc( _r.type );
}

void MemoryDiff::Removal::Write( std::streambuf* sb ) const
{
	sb->sputc( '\01' );
	MyIntPut( sb, _b );
}

void MemoryDiff::DetailChange::Write( std::streambuf* sb ) const
{
	sb->sputc( '\02' );
	MyIntPut( sb, _r.base );
	MyIntPut( sb, _r.size );
	sb->sputc( _r.type );
}

template< class StreamBuf >
void MemoryDiff::Write( StreamBuf* sb ) const
{
	sb->sputn( "md", 3 );
	sb->sputc( sizeof( size_t ) );

	for( Changes::const_iterator i = _changes.begin(); i != _changes.end(); ++i )
	{
		DoWrite( i->get(), sb );
	}
	sb->sputc( '\xf0' );
}

template< class StreamBuf >
void MemoryDiff::Read( StreamBuf* sb )
{
	typedef typename StreamBuf::traits_type traits_type;

	char b[4];

	if( sb->sgetn( b, 4 ) != 4 )
		throw ReadError( "MemoryDiff signature is incomplete" );

	if( memcmp( b, "md", 3) != 0 )
		throw ReadError( "MemoryDiff signature is incorrect" );

	size_t sz = b[3];

	typename traits_type::int_type j;

	while( (j = sb->sbumpc()) != traits_type::eof() && j != 0xf0 )
	{
		Region r;

		switch( j )
		{
		case 0:
			MyIntGet( sb, r.base, sz );
			MyIntGet( sb, r.size, sz );
			r.type = static_cast< Region::Type >( sb->sbumpc() );
			_changes.push_back( Changes::value_type( new Addition( r ) ) );
			break;

		case 1:
			MyIntGet( sb, r.base, sz );
			_changes.push_back( Changes::value_type( new Removal( r.base ) ) );
			break;

		case 2:
			MyIntGet( sb, r.base, sz );
			MyIntGet( sb, r.size, sz );
			r.type = static_cast< Region::Type >( sb->sbumpc() );
			_changes.push_back( Changes::value_type( new DetailChange( r ) ) );
			break;
		}
	}
}

template void MemoryDiff::Write( std::streambuf* ) const;
template void MemoryDiff::Read( std::streambuf* );

}
