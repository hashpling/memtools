#include "mminfo.h"
#include <iostream>
#include <fstream>

int main( int argc, char* argv[] )
{
	MemMon::MemoryMap m;
	std::filebuf buf;
	std::streambuf* bufptr = &buf;

	for( int i = 1; i < argc; ++i )
	{
		buf.open( argv[i], std::ios_base::in | std::ios_base::binary );
		if( buf.is_open() )
		{
			m.Clear( 0 );
			m.Read( bufptr );

			for( MemMon::RegionList::const_iterator i = m.GetBlockList().begin(); i != m.GetBlockList().end(); ++i )
			{
				std::cout << "Base = " << i->base << " Size = " << i->size << " Type = " << (int)i->type << '\n';
			}
		}
	}

	return 0;
}
