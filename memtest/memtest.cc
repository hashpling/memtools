#include <iostream>
#include "mminfo.h"
#include "hshgtest.h"

namespace
{

void DiffAddMid()
{
	// Add new block in the middle
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

	HSHG_ASSERT( c.size() == 1 );

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::addition );

	const MemMon::Region* rr = &cit->second.second;

	HSHG_ASSERT( rr->base == 10 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	HSHG_ASSERT( crev.size() == 1 );

	cit = crev.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::removal );

	rr = &cit->second.first;

	HSHG_ASSERT( rr->base == 10 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );
}

void DiffAddStart()
{
	// Add new block in the middle
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	MemMon::Region r;

	r.base = 0;
	r.type = MemMon::Region::free;
	r.size = 30;

	m1.AddBlock( r );

	r.size = 10;
	r.type = MemMon::Region::committed;
	m2.AddBlock( r );

	r.size = 20;
	r.base = 10;
	r.type = MemMon::Region::free;
	m2.AddBlock( r );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	HSHG_ASSERT( c.size() == 1 );

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::addition );

	const MemMon::Region* rr = &cit->second.second;

	HSHG_ASSERT( rr->base == 0 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	HSHG_ASSERT( crev.size() == 1 );

	cit = crev.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::removal );

	rr = &cit->second.first;

	HSHG_ASSERT( rr->base == 0 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );
}

void DiffAddEnd()
{
	// Add new block in the middle
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	MemMon::Region r;

	r.base = 0;
	r.type = MemMon::Region::free;
	r.size = 30;

	m1.AddBlock( r );

	r.size = 20;
	m2.AddBlock( r );

	r.size = 10;
	r.base = 20;
	r.type = MemMon::Region::committed;
	m2.AddBlock( r );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	HSHG_ASSERT( c.size() == 1 );

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::addition );

	const MemMon::Region* rr = &cit->second.second;

	HSHG_ASSERT( rr->base == 20 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	HSHG_ASSERT( crev.size() == 1 );

	cit = crev.begin();

	HSHG_ASSERT( cit->first == MemMon::MemoryDiff::removal );

	rr = &cit->second.first;

	HSHG_ASSERT( rr->base == 20 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == MemMon::Region::committed );
}

void PatchAddMid()
{
	// Add new block in the middle
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

	r.base = 10;
	r.type = MemMon::Region::committed;
	r.size = 10;
	MemMon::MemoryDiff d;
	d.AppendAddition( r );

	d.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

}

HSHG_BEGIN_TESTS
HSHG_TEST_ENTRY( DiffAddMid )
HSHG_TEST_ENTRY( DiffAddStart )
HSHG_TEST_ENTRY( DiffAddEnd )
HSHG_TEST_ENTRY( PatchAddMid )
HSHG_END_TESTS

HSHG_TEST_MAIN
