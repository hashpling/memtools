#include "mminfo.h"
#include <fstream>
#include <iostream>

int main( int argc, char* argv[] )
{
	if( argc != 4 )
	{
		std::cerr << "Syntax: memdiff <before> <after> <output>";
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
		std::cerr << "Patch applied incorrectly\n";
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
