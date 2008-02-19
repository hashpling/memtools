#ifndef MEMORYMAP_H
#define MEMORYMAP_H

// Copyright (c) 2007,2008 Charles Bailey

#include "mminfo.h"
#include <utility>

namespace MemMon
{

class MemoryMap
{
public:
	MemoryMap() {}

	template< class StreamBuf >
	void Write( StreamBuf* ) const;

	template< class StreamBuf >
	void Read( StreamBuf* );

	bool operator==( const MemoryMap& other ) const
	{
		return _blocklist == other._blocklist && _ts == other._ts;
	}
	bool operator!=( const MemoryMap& other ) const { return !(*this == other); }

	const RegionList& GetBlockList() const { return _blocklist; }
	const FreeList& GetFreeList() const { return _freelist; }

	size_t GetFreeTotal() const { return _total_free; }
	size_t GetCommitTotal() const { return _total_commit; }
	size_t GetReserveTotal() const { return _total_reserve; }

	void Clear( size_t freecount = 50 );
	void AddBlock( const Region& r );

	void RecalcFreeList();

	RegionList& GetBlockListRef() { return _blocklist; }

	void Swap( MemoryMap& other );

	void Stamp() { _ts = Timestamp::now(); }

	const Timestamp& GetTimestamp() const { return _ts; }

private:
	MemoryMap( const MemoryMap& );
	MemoryMap& operator=( const MemoryMap& );

	void UpdateFreeList( const Region& r, const Region* modified );
	void PartialClear();

	RegionList _blocklist;
	FreeList _freelist;

	size_t _total_free;
	size_t _total_commit;
	size_t _total_reserve;

	Timestamp _ts;
};

template< class Stream >
inline Stream& operator<<( Stream& os, const MemoryMap& mem )
{
	mem.Write( os.rdbuf() );
	return os;
}

template< class Stream >
inline Stream& operator>>( Stream& is, MemoryMap& mem)
{
	mem.Read( is.rdbuf() );
	return is;
}

}

namespace std
{
	template<> inline void swap( MemMon::MemoryMap& l, MemMon::MemoryMap& r ) { l.Swap( r ); }
}

#endif//MEMORYMAP_H
