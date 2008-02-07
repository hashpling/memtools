#include <iostream>
#include "mminfo.h"

int main()
{
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	MemMon::Region r;

	r.base = 0;
	r.type = MemMon::Region::free;
	r.size = 30;

	m1.AddBlock( r );

	r.size = 10;
	m2.AddBlock( r );

	r.base = 10;
	r.type = MemMon::Region::committed;
	m2.AddBlock( r );

	r.base = 20;
	r.type = MemMon::Region::free;
	m2.AddBlock( r );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	return 0;
}
