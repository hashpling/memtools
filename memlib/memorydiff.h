#ifndef MEMORYDIFF_H
#define MEMORYDIFF_H

// Copyright (c) 2008 Charles Bailey

#include "mminfo.h"
#include <memory>

namespace MemMon
{

class MemoryMap;

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
		virtual void Write( std::streambuf* sb ) const = 0;
	};

	class Addition : public MemoryDiff::Change
	{
	public:
		explicit Addition( const Region& r ) : _r( r ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new Addition( _r ); }
		void Write( std::streambuf* sb ) const;

		const Region& GetRegion() const { return _r; }

	private:
		Addition( const Addition& );
		Addition& operator=( const Addition& );

		Region _r;
	};

	class Removal : public MemoryDiff::Change
	{
	public:
		explicit Removal( const size_t& b ) : _b( b ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new Removal( _b ); }
		void Write( std::streambuf* sb ) const;

		size_t GetBase() const { return _b; }

	private:
		Removal( const Removal& );
		Removal& operator=( const Removal& );

		size_t _b;
	};

	class DetailChange : public MemoryDiff::Change
	{
	public:
		explicit DetailChange( const Region& r ) : _r( r ) {}
		void Apply( RegionList&, RegionList::iterator& ) const;
		Change* Clone() const { return new DetailChange( _r ); }
		void Write( std::streambuf* sb ) const;

		const Region& GetRegion() const { return _r; }

	private:
		DetailChange( const DetailChange& );
		DetailChange& operator=( const DetailChange& );

		Region _r;
	};

	typedef std::vector< std::unique_ptr< Change > > Changes;

	const Changes& GetChanges() const { return _changes; }

	void Append( Change* c ) { _changes.push_back( Changes::value_type( c ) ); }

	const TimeInterval& GetTimeInterval() const { return _ti; }

private:
	Changes _changes;
	TimeInterval _ti;
};

}

#endif//MEMORYDIFF_H
