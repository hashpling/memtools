#include "mminfo.h"
#include <fstream>
#include <iostream>

int main( int argc, char* argv[] )
{
	if( argc != 4 )
	{
		std::cerr << "Syntax: memdiff <before> <after> <output>\n";
		return EXIT_FAILURE;
	}

	MemMon::MemoryMap mm1, mm2;

	std::filebuf buf;

	buf.open( argv[1], std::ios_base::in | std::ios_base::binary );
	if( !buf.is_open() )
	{
		std::cerr << "Failed to open " << argv[1] << '\n';
		return EXIT_FAILURE;
	}
	mm1.Read( &buf );
	buf.close();

	buf.open( argv[2], std::ios_base::in | std::ios_base::binary );
	if( !buf.is_open() )
	{
		std::cerr << "Failed to open " << argv[2] << '\n';
		return EXIT_FAILURE;
	}
	mm2.Read( &buf );
	buf.close();

	MemMon::MemoryDiff md( mm1, mm2 );

	try
	{
		md.Apply( mm1 );
	}
	catch( ... )
	{
		std::cerr << "Patch failed to apply\n";
		return EXIT_FAILURE;
	}

	if( mm1 != mm2 )
	{
		size_t s = std::min( mm1.GetBlockList().size(), mm2.GetBlockList().size() );
		std::pair< MemMon::RegionList::const_iterator, MemMon::RegionList::const_iterator > p =
			std::mismatch( mm1.GetBlockList().begin(), mm1.GetBlockList().begin() + s, mm2.GetBlockList().begin() );

		std::cerr << "Patch applied incorrectly\n";

		std::cerr << "Region base = " << p.second->base << " size = " << p.second->size << '\n';
		std::cerr << "Region base = " << p.first->base << " size = " << p.first->size << '\n';

		return EXIT_FAILURE;
	}

	buf.open( argv[3], std::ios_base::out | std::ios_base::binary );
	if( !buf.is_open() )
	{
		std::cerr << "Failed to open " << argv[3] << '\n';
		return EXIT_FAILURE;
	}
	md.Write( &buf );
	buf.close();

	return EXIT_SUCCESS;
}
