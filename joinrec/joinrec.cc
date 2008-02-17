#include "mminfo.h"
#include <fstream>
#include <sstream>

int main( int argc, char* argv[])
{
	if( argc > 2 )
	{
		std::filebuf in;
		std::filebuf out;

		std::streambuf* inptr = &in;
		std::streambuf* outptr = &out;

		out.open( argv[ argc - 1 ], std::ios_base::out | std::ios_base::binary );
		if( !out.is_open() )
			return EXIT_FAILURE;

		MemMon::MemoryMap m, m2;

		in.open( argv[1], std::ios_base::in | std::ios_base::binary );
		if( !in.is_open() )
			return EXIT_FAILURE;

		m.Read( inptr );
		m.Write( outptr );

		in.close();

		for( int i = 2; i < argc - 1; ++i )
		{
			in.open( argv[i], std::ios_base::in | std::ios_base::binary );

			if( in.is_open() )
			{
				m2.Read( inptr );
				MemMon::MemoryDiff md( m, m2 );

				md.Write( outptr );

				std::swap( m, m2 );

				in.close();
			}
		}
	}
	return 0;
}
