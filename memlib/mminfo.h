#ifndef MMINFO_H
#define MMINFO_H

// Copyright (c) 2008 Charles Bailey

#include <streambuf>
#include <vector>
#include <utility>
#include <exception>
#include "mmvalueptr.h"

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

class ReadError : public std::exception
{
public:
	ReadError( const char* msg ) : _msg( msg ) {}
	virtual ~ReadError() throw() {}
	virtual const char* what() const throw() { return _msg.c_str(); }

private:
	std::string _msg;
};

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

	class Change
	{
	public:
		virtual ~Change() {}
		virtual void Apply( RegionList&, RegionList::iterator& ) const = 0;
		virtual Change* Clone() const = 0;
	};

	class Addition : public MemoryDiff::Change
	{
	public:
		explicit Addition( const Region& r ) : _r( r ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new Addition( _r ); }

		const Region& GetRegion() const { return _r; }

	private:
		Addition( const Addition& );
		Addition& operator=( const Addition& );

		Region _r;
	};

	class Removal : public MemoryDiff::Change
	{
	public:
		explicit Removal( const Region& r ) : _r( r ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new Removal( _r ); }

		const Region& GetRegion() const { return _r; }

	private:
		Removal( const Removal& );
		Removal& operator=( const Removal& );

		Region _r;
	};

	class DetailChange : public MemoryDiff::Change
	{
	public:
		explicit DetailChange( const Region& b, const Region& a ) : _b( b ), _a( a ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new DetailChange( _b, _a ); }

		const Region& GetBefore() const { return _b; }
		const Region& GetAfter() const { return _a; }

	private:
		DetailChange( const DetailChange& );
		DetailChange& operator=( const DetailChange& );

		Region _b;
		Region _a;
	};

	typedef std::vector< ValuePtr< Change, Cloner > > Changes;

	const Changes& GetChanges() const { return _changes; }

	void Append( Change* c ) { _changes.push_back( Changes::value_type( c ) ); }

private:
	Changes _changes;
};

}

namespace std
{
	template<> inline void swap( MemMon::MemoryMap& l, MemMon::MemoryMap& r ) { l.Swap( r ); }
}

#endif//MMINFO_H
