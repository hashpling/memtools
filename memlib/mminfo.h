#ifndef MMINFO_H
#define MMINFO_H

// Copyright (c) 2008 Charles Bailey

#include <streambuf>
#include <vector>
#include <utility>
#include <exception>
#include "mmvalueptr.h"

#ifndef _WIN32
#include <stdint.h>
#endif

namespace MemMon
{

// Basic type to throw from constructors
template< class T >
struct ConstructorFailure {};

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

#ifdef _WIN32
	typedef __int64 mmint64;
#else
	typedef int_least64_t mmint64;
#endif

class Timestamp;

class TimeInterval
{
public:
	friend class Timestamp;
	friend TimeInterval operator-( const Timestamp&, const Timestamp& );

	TimeInterval() : _msec( 0 ) {}
	explicit TimeInterval( mmint64 ms ) : _msec( ms ) {}

	template< class StreamBuf >
	void Write( StreamBuf* ) const;

	template< class StreamBuf >
	void Read( StreamBuf* );

private:
	mmint64 _msec;
};

class Timestamp
{
public:
	Timestamp() : _msec( 0 ) {}
	explicit Timestamp( mmint64 ms ) : _msec( ms ) {}
	explicit Timestamp( const char* utcstring );

	static Timestamp now();

	template< class StreamBuf >
	void Write( StreamBuf* ) const;

	template< class StreamBuf >
	void Read( StreamBuf* );

	long seconds() const { return static_cast<long>( _msec / 1000 ); }
	long milliseconds() const { return static_cast<long>( _msec % 1000 ); }

	Timestamp& operator+=( const TimeInterval& ti )
	{
		_msec += ti._msec;
		return *this;
	}

	Timestamp& operator-=( const TimeInterval& ti )
	{
		_msec -= ti._msec;
		return *this;
	}

	friend TimeInterval operator-( const Timestamp&, const Timestamp& );

	bool operator==( const Timestamp& other ) const
	{
		return _msec == other._msec;
	}

	bool operator!=( const Timestamp& other ) const
	{
		return !(*this == other);
	}

	std::string GetUTCString() const;

private:
	mmint64 _msec;
};

inline TimeInterval operator-( const Timestamp& l, const Timestamp& r )
{
	return TimeInterval( l._msec - r._msec );
}

inline Timestamp operator+( Timestamp ts, const TimeInterval& ti )
{
	return ts += ti;
}

inline Timestamp operator+( const TimeInterval& ti, Timestamp ts )
{
	return ts += ti;
}

inline Timestamp operator-( Timestamp ts, const TimeInterval& ti )
{
	return ts -= ti;
}

template< class Stream >
inline Stream& operator<<( Stream& os, const Timestamp& ts )
{
	ts.Write( os.rdbuf() );
	return os;
}

template< class Stream >
inline Stream& operator>>( Stream& is, Timestamp& ts)
{
	ts.Read( is.rdbuf() );
	return is;
}

}

#endif//MMINFO_H
