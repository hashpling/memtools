#include "memorymap.h"
#include "memorydiff.h"
#include <iostream>
#include <fstream>
#include <iomanip>

namespace
{
	void PrintMem( const MemMon::MemoryMap& m, bool summary )
	{
		if( !summary )
		{
			for( MemMon::RegionList::const_iterator i = m.GetBlockList().begin(); i != m.GetBlockList().end(); ++i )
			{
				std::cout << "Base = " << i->base << " Size = " << i->size << " Type = " << (int)i->type << '\n';
			}
			std::cout << '\n';
		}

		std::cout << m.GetTimestamp().GetAsString().c_str()
			<< '\t' << m.GetFreeTotal() << "\t" << m.GetReserveTotal()
			<< '\t' << m.GetCommitTotal() << '\t' << m.GetFreeList().front().size
			<< '\t' << std::setbase( std::ios_base::hex ) << m.GetFreeList().front().base
			<< std::setbase( std::ios_base::dec ) << std::endl;
	}
}

int main( int argc, char* argv[] )
{
	bool noopts = false;
	bool summary = false;
	MemMon::MemoryMap m;
	std::filebuf buf;
	std::streambuf* bufptr = &buf;

	for( int i = 1; i < argc; ++i )
	{
		if( !noopts )
		{
			if( strcmp( argv[i], "--" ) == 0 )
			{
				noopts = true;
				continue;
			}
			else if( strcmp( argv[i], "-s" ) == 0 )
			{
				summary = true;
				continue;
			}
		}

		buf.open( argv[i], std::ios_base::in | std::ios_base::binary );
		if( buf.is_open() )
		{
			m.Clear( 1 );
			m.Read( bufptr );

			PrintMem( m, summary );

			while( !std::filebuf::traits_type::eq_int_type( buf.sgetc(), std::filebuf::traits_type::eof() ) )
			{
				MemMon::MemoryDiff md;
				md.Read( bufptr );
				md.Apply( m );

				PrintMem( m, summary );
			}

			buf.close();
		}
	}

	return 0;
}
