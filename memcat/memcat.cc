#include "memorymap.h"
#include <iostream>
#include <fstream>
#include <iomanip>

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
			m.Clear( 1 );
			m.Read( bufptr );

			for( MemMon::RegionList::const_iterator i = m.GetBlockList().begin(); i != m.GetBlockList().end(); ++i )
			{
				std::cout << "Base = " << i->base << " Size = " << i->size << " Type = " << (int)i->type << '\n';
			}

			std::cout << "\nGenerated at " << m.GetTimestamp().GetUTCString().c_str() << '\n';

			std::cout << "\nTotal free = " << m.GetFreeTotal() << " Total reserved = " << m.GetReserveTotal()
				<< " Total committed = " << m.GetCommitTotal() << '\n';

			std::cout << " Largest free block = "
				<< m.GetFreeList().front().size << " @ " << std::setbase( std::ios_base::hex ) << m.GetFreeList().front().base << std::endl;
		}
	}

	return 0;
}
