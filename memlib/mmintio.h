#ifndef MMINTIO_H
#define MMINTIO_H

namespace MemMon
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

#endif//MMINTIO_H
