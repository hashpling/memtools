#ifndef MMINFO_H
#define MMINFO_H

// Copyright (c) 2008 Charles Bailey

#include <streambuf>
#include <vector>
#include <utility>

namespace MemMon
{

struct FreeRegion
{
	FreeRegion( size_t b, size_t s ) : base( b ), size( s ) {}
	FreeRegion() {}

	size_t base;
	size_t size;
};

struct Region : public FreeRegion
{
	enum Type
	{
		  free
		, reserved
		, committed
	};

	Region( size_t b, size_t s, Type t ) : FreeRegion( b , s ), type( t ) {}
	Region() {}

	Type type;
};

inline bool operator==(const Region& lhs, const Region& rhs)
{
	return lhs.base == rhs.base && lhs.size == rhs.size && lhs.type == rhs.type;
}

inline bool operator==(const FreeRegion& lhs, const FreeRegion& rhs)
{
	return lhs.size == rhs.size && (lhs.size == 0 || lhs.base == rhs.base);
}

typedef std::vector< Region > RegionList;
typedef std::vector< FreeRegion > FreeList;

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
		return _blocklist == other._blocklist && _freelist == other._freelist;
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

class MemoryDiff
{
public:
	MemoryDiff() {}
	MemoryDiff( const MemoryMap& before, const MemoryMap& after );

	void Apply( MemoryMap& target ) const;
	void ReverseApply( MemoryMap& target ) const;

	template< class StreamBuf >
	void Write( StreamBuf* ) const;

	template< class StreamBuf >
	void Read( StreamBuf* );

	enum ChangeType
	{
		  addition
		, removal
		, change
	};

	struct Change
	{
		Change() {}
		Change( ChangeType ct, const std::pair< Region, Region >& rp ) : first( ct ), second( rp ) {}
		ChangeType first;
		std::pair< Region, Region > second;
		bool operator==( const Change& other ) const
		{
			switch( first )
			{
			case addition:
				return other.first == addition && second.second == other.second.second;
			case removal:
				return other.first == removal && second.first == other.second.first;
			case change:
				return other.first == change && second.first == other.second.first && second.second == other.second.second;
			}
			return false;
		}
		bool operator!=( const Change& other ) const { return !(*this == other); }
	};

	typedef std::vector< Change > Changes;

	const Changes& GetChanges() const { return _changes; }

	void AppendAddition( const Region& r )
	{
		_changes.push_back( Change( MemoryDiff::addition, std::make_pair( Region(), r ) ) );
	}
	void AppendRemoval( const Region& r )
	{
		_changes.push_back( Change( MemoryDiff::removal, std::make_pair( r, Region() ) ) );
	}
	void AppendChange( const Region& r1, const Region& r2 )
	{
		_changes.push_back( Change( MemoryDiff::change, std::make_pair( r1, r2 ) ) );
	}

	bool operator==( const MemoryDiff& other ) const
	{
		return _changes == other._changes;
	}

	bool operator!=( const MemoryDiff& other ) const { return !(*this == other); }

private:
	Changes _changes;
};

}

namespace std
{
	template<> inline void swap( MemMon::MemoryMap& l, MemMon::MemoryMap& r ) { l.Swap( r ); }
}

#endif//MMINFO_H
