#include "mminfo.h"
#include <fstream>
#include <sstream>

int main( int argc, char* argv[] )
{
	for( int i = 1; i < argc; ++i )
	{
		int count = 0;
		std::filebuf buf;
		buf.open( argv[i], std::ios_base::in | std::ios_base::binary );
		if( buf.is_open() )
		{
			MemMon::MemoryMap mm;
			mm.Read( &buf );

			while( !std::filebuf::traits_type::eq_int_type( buf.sgetc(), std::filebuf::traits_type::eof() ) )
			{
				MemMon::MemoryDiff md;
				md.Read( &buf );
				md.Apply( mm );

				std::ostringstream fnamemaker;
				fnamemaker << argv[i] << "_SPLIT_" << ++count;

				std::filebuf out;
				out.open( fnamemaker.str().c_str(), std::ios_base::out | std::ios_base::binary );

				if( out.is_open() )
				{
					mm.Write( &out );
				}
			}
		}
	}
	return 0;
}
