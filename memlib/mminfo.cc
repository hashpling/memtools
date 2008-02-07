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

template<typename charT, typename traits, typename intT>
void MyIntPut( basic_streambuf<charT, traits>* sb, intT toput )
{
	for (unsigned i = 0; i < sizeof(intT) - 1; ++i)
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
	for (unsigned i = 0; i < sizeof(intT); ++i)
	{
		toget |= intT(sb->sbumpc()) << (i * 8);
	}
}

}

template<typename charT, typename traits>
void MemoryMap::Write(basic_streambuf<charT, traits>* sb) const
{
	sb->sputn("V1", 3);
	sb->sputc(charT(sizeof(size_t)));

	size_t m = (size_t)-1;

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

	sb->sputc('\xf0');
}

template<typename charT, typename traits>
void MemoryMap::Read(basic_streambuf<charT, traits>* sb)
{
	stringbuf s;
	charT t;
	while( (t = sb->sbumpc()) != '\0' && t != traits::eof() )
		s.sputc(t);

	if (s.str() != "V1")
		throw ios_base::failure("This file is not a valid Address Space Monitor dump.");

	if (sb->sbumpc() != sizeof(size_t))
		throw ios_base::failure("This file is not compatible with this version of Address Space Monitor as it was saved by a version compiled for a different architecture.");

	Clear();

	size_t m = (size_t)-1;
	while( (t = sb->sbumpc()) != '\xf0' && t != traits::eof() )
	{
		Region r;
		r.type = static_cast< Region::Type >( t & 0xf );

		if( (t & 0x40) != 0 )
			MyIntGet(sb, r.base);
		else
			r.base = m;

		MyIntGet(sb, r.size);

		m = r.base + r.size;

		AddBlock( r );
	}
}

template void MemoryMap::Read(streambuf* sb);
template void MemoryMap::Write(streambuf* sb) const;

void MemoryMap::Clear( size_t freecount )
{
	_freelist.resize( freecount );
	_blocklist.clear();

	_total_free = 0;
	_total_reserve = 0;
	_total_commit = 0;

	for( FreeList::iterator i = _freelist.begin(); i != _freelist.end(); ++i )
	{
		i->size = 0;
	}
}

void MemoryMap::AddBlock( const Region& r )
{
	_blocklist.push_back( r );

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

	if( r.type == 0 && _freelist.back().size < r.size)
	{
		int j;

		for(j = int(_freelist.size()) - 2; j >= 0; --j)
		{
			if (_freelist[j].size >= r.size) break;
			_freelist[j+1].size = _freelist[j].size;
			_freelist[j+1].base = _freelist[j].base;
		}

		_freelist[j+1].size = r.size;
		_freelist[j+1].base = r.base;
	}
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
				bool bNewPreserved = false;

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
				}

				++ait;
			}
			else if ( ait->type != bit->type )
			{
				AppendChange( *bit++, *ait++ );
			}
		}
	}
}

}
