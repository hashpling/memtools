#ifndef MMINTIO_H
#define MMINTIO_H

// Copyright (c) 2007,2008 Charles Bailey

namespace MemMon
{

template< class StreamBuf, class intT >
void IntPut( StreamBuf* sb, intT toput, size_t width = sizeof(intT) )
{
	typedef typename StreamBuf::char_type charT;
	for (unsigned i = 0; i < width - 1; ++i)
	{
		sb->sputc( static_cast< charT >( toput & 0xff ) );
		toput >>= 8;
	}
	sb->sputc( static_cast< charT >( toput & 0xff ) );
}

template< class StreamBuf, class intT >
void IntGet( StreamBuf* sb, intT& toget, size_t width = sizeof(intT) )
{
	toget = 0;
	for (unsigned i = 0; i < width; ++i)
	{
		toget |= static_cast< intT >( sb->sbumpc() ) << (i * 8);
	}
}

}

#endif//MMINTIO_H
