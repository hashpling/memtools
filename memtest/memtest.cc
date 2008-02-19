#include <iostream>
#include <sstream>
#include "memorymap.h"
#include "memorydiff.h"
#include "hshgtest.h"

namespace
{

using MemMon::Region;

void DiffAddMid()
{
	// Add new block in the middle
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 10, Region::free ) );
	m2.AddBlock( Region( 10, 10, Region::committed ) );
	m2.AddBlock( Region( 20, 10, Region::free ) );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	HSHG_ASSERT( c.size() == 1 );

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	const MemMon::MemoryDiff::Addition* pAdd = dynamic_cast< const MemMon::MemoryDiff::Addition* >( cit->get() );
	HSHG_ASSERT( pAdd != NULL );

	const Region* rr = &pAdd->GetRegion();

	HSHG_ASSERT( rr->base == 10 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == Region::committed );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	HSHG_ASSERT( crev.size() == 1 );

	cit = crev.begin();

	const MemMon::MemoryDiff::Removal* pRem = dynamic_cast< const MemMon::MemoryDiff::Removal* >( cit->get() );
	HSHG_ASSERT( pRem != NULL);

	HSHG_ASSERT( pRem->GetBase() == 10 );
}

void DiffAddStart()
{
	// Add new block at the start
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 10, Region::committed ) );
	m2.AddBlock( Region( 10, 20, Region::free ) );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	HSHG_ASSERT( cit != c.end() );

	const MemMon::MemoryDiff::Addition* pAdd = dynamic_cast< const MemMon::MemoryDiff::Addition* >( cit->get() );
	HSHG_ASSERT( pAdd != NULL );

	const Region* rr = &pAdd->GetRegion();

	HSHG_ASSERT( rr->base == 0 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == Region::committed );

	HSHG_ASSERT( ++cit == c.end() );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	cit = crev.begin();

	HSHG_ASSERT( cit != crev.end() );

	const MemMon::MemoryDiff::DetailChange* pChg = dynamic_cast< const MemMon::MemoryDiff::DetailChange* >( cit->get() );
	HSHG_ASSERT( pChg != NULL);

	rr = &pChg->GetRegion();

	HSHG_ASSERT( rr->base == 0 );
	HSHG_ASSERT( rr->size == 30 );
	HSHG_ASSERT( rr->type == Region::free );

	HSHG_ASSERT( ++cit == crev.end() );
}

void DiffAddEnd()
{
	// Add new block at the end
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 20, Region::free ) );
	m2.AddBlock( Region( 20, 10, Region::committed ) );

	MemMon::MemoryDiff d( m1, m2 );

	const MemMon::MemoryDiff::Changes& c = d.GetChanges();

	MemMon::MemoryDiff::Changes::const_iterator cit = c.begin();

	HSHG_ASSERT( cit != c.end() );

	const MemMon::MemoryDiff::Addition* pAdd = dynamic_cast< const MemMon::MemoryDiff::Addition* >( cit->get() );
	HSHG_ASSERT( pAdd != NULL );

	const Region* rr = &pAdd->GetRegion();

	HSHG_ASSERT( rr->base == 20 );
	HSHG_ASSERT( rr->size == 10 );
	HSHG_ASSERT( rr->type == Region::committed );

	HSHG_ASSERT( ++cit == c.end() );

	MemMon::MemoryDiff drev( m2, m1 );

	const MemMon::MemoryDiff::Changes& crev = drev.GetChanges();

	cit = crev.begin();

	HSHG_ASSERT( cit != crev.end() );

	const MemMon::MemoryDiff::DetailChange* pChg = dynamic_cast< const MemMon::MemoryDiff::DetailChange* >( cit->get() );
	HSHG_ASSERT( pChg != NULL );

	rr = &pChg->GetRegion();

	HSHG_ASSERT( rr->base == 0 );
	HSHG_ASSERT( rr->size == 30 );
	HSHG_ASSERT( rr->type == Region::free );

	HSHG_ASSERT( ++cit == crev.end() );
}

void PatchAddMid()
{
	// Add new block in the middle
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 10, Region::free ) );
	m2.AddBlock( Region( 10, 10, Region::committed ) );
	m2.AddBlock( Region( 20, 10, Region::free ) );

	MemMon::MemoryDiff d;
	d.Append( new MemMon::MemoryDiff::Addition( Region( 10, 10, Region::committed ) ) );

	d.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void PatchAddStart()
{
	// Add new block in the start
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 10, Region::committed ) );
	m2.AddBlock( Region( 10, 20, Region::free ) );

	MemMon::MemoryDiff d;
	d.Append( new MemMon::MemoryDiff::Addition( Region( 0, 10, Region::committed ) ) );

	d.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void PatchAddEnd()
{
	// Add new block in the end
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 30, Region::free ) );

	m2.AddBlock( Region( 0, 20, Region::free ) );
	m2.AddBlock( Region( 20, 10, Region::committed ) );

	MemMon::MemoryDiff d;
	d.Append( new MemMon::MemoryDiff::Addition( Region( 20, 10, Region::committed ) ) );

	d.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void PatchAddSplit()
{
	MemMon::MemoryMap m1, m2;

	m1.Clear( 10 );
	m2.Clear( 10 );

	m1.AddBlock( Region( 0, 15, Region::free ) );
	m1.AddBlock( Region( 15, 15, Region::reserved ) );

	m2.AddBlock( Region( 0, 10, Region::free ) );
	m2.AddBlock( Region( 10, 10, Region::committed ) );
	m2.AddBlock( Region( 20, 10, Region::reserved ) );

	MemMon::MemoryDiff d;
	d.Append( new MemMon::MemoryDiff::Addition( Region( 10, 10, Region::committed ) ) );

	d.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void DiffIoTrip()
{
	MemMon::MemoryDiff d;
	d.Append( new MemMon::MemoryDiff::Addition( Region( 10, 10, Region::committed ) ) );
	d.Append( new MemMon::MemoryDiff::DetailChange( Region( 30, 10, Region::reserved ) ) );
	d.Append( new MemMon::MemoryDiff::Removal( 50 ) );

	std::stringbuf buf;
	std::streambuf* bufptr = &buf;

	d.Write( bufptr );
	buf.pubsync();

	MemMon::MemoryDiff d2;

	d2.Read( bufptr );

	MemMon::MemoryDiff::Changes::const_iterator cit = d2.GetChanges().begin();
	const MemMon::MemoryDiff::Changes::const_iterator cend = d2.GetChanges().end();

	HSHG_ASSERT( cit != cend );

	const MemMon::MemoryDiff::Addition* pAdd = dynamic_cast< const MemMon::MemoryDiff::Addition* >( cit->get() );
	HSHG_ASSERT( pAdd != NULL );

	HSHG_ASSERT( pAdd->GetRegion() == Region( 10, 10, Region::committed ) );

	HSHG_ASSERT( ++cit != cend );

	const MemMon::MemoryDiff::DetailChange* pChg = dynamic_cast< const MemMon::MemoryDiff::DetailChange* >( cit->get() );
	HSHG_ASSERT( pChg != NULL );

	HSHG_ASSERT( pChg->GetRegion() == Region( 30, 10, Region::reserved ) );

	HSHG_ASSERT( ++cit != cend );

	const MemMon::MemoryDiff::Removal* pRem = dynamic_cast< const MemMon::MemoryDiff::Removal* >( cit->get() );
	HSHG_ASSERT( pRem != NULL );

	HSHG_ASSERT( pRem->GetBase() == 50 );

	HSHG_ASSERT( ++cit == cend );
}

void MergeAdjacentFree()
{
	MemMon::MemoryMap m;
	m.Clear( 3 );

	m.AddBlock( Region( 0, 5, Region::free ) );
	m.AddBlock( Region( 5, 5, Region::committed ) );
	m.AddBlock( Region( 10, 7, Region::free ) );
	m.AddBlock( Region( 17, 3, Region::committed ) );
	m.AddBlock( Region( 20, 6, Region::free ) );
	m.AddBlock( Region( 26, 4, Region::free ) );

	const MemMon::FreeList& fl = m.GetFreeList();

	HSHG_ASSERT( fl.size() == 3 );
	HSHG_ASSERT( fl[0].base == 20 && fl[0].size == 10 );
	HSHG_ASSERT( fl[1].base == 10 && fl[1].size == 7 );
	HSHG_ASSERT( fl[2].base == 0 && fl[2].size == 5 );

	m.AddBlock( Region( 30, 10, Region::committed ) );
	m.AddBlock( Region( 40, 4, Region::free ) );

	HSHG_ASSERT( fl[2].base == 0 && fl[2].size == 5 );

	m.AddBlock( Region( 44, 4, Region::free ) );

	HSHG_ASSERT( fl[2].base == 10 && fl[2].size == 7 );
	HSHG_ASSERT( fl[1].base == 40 && fl[1].size == 8 );
}

void MoveBoundary1()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 5, Region::free ) );
	m1.AddBlock( Region( 5, 5, Region::committed ) );

	m2.AddBlock( Region( 0, 7, Region::free ) );
	m2.AddBlock( Region( 7, 3, Region::committed ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void MoveBoundary2()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 5, Region::free ) );
	m1.AddBlock( Region( 5, 5, Region::committed ) );

	m2.AddBlock( Region( 0, 3, Region::free ) );
	m2.AddBlock( Region( 3, 7, Region::committed ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void MoveDoubleBoundary()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 3, Region::free ) );
	m1.AddBlock( Region( 3, 2, Region::committed ) );
	m1.AddBlock( Region( 5, 5, Region::free ) );

	m2.AddBlock( Region( 0, 5, Region::free ) );
	m2.AddBlock( Region( 5, 2, Region::committed ) );
	m2.AddBlock( Region( 7, 3, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ExpandBothEnds()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 4, Region::free ) );
	m1.AddBlock( Region( 4, 2, Region::committed ) );
	m1.AddBlock( Region( 6, 4, Region::free ) );

	m2.AddBlock( Region( 0, 3, Region::free ) );
	m2.AddBlock( Region( 3, 4, Region::committed ) );
	m2.AddBlock( Region( 7, 3, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ContractBothEnds()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 3, Region::free ) );
	m1.AddBlock( Region( 3, 4, Region::committed ) );
	m1.AddBlock( Region( 7, 3, Region::free ) );

	m2.AddBlock( Region( 0, 4, Region::free ) );
	m2.AddBlock( Region( 4, 2, Region::committed ) );
	m2.AddBlock( Region( 6, 4, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ContractRemove()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 4, Region::reserved ) );
	m1.AddBlock( Region( 4, 2, Region::committed ) );
	m1.AddBlock( Region( 6, 2, Region::reserved ) );
	m1.AddBlock( Region( 8, 2, Region::committed ) );
	m1.AddBlock( Region( 10, 2, Region::free ) );

	m2.AddBlock( Region( 0, 3, Region::reserved ) );
	m2.AddBlock( Region( 3, 7, Region::committed ) );
	m2.AddBlock( Region( 10, 2, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ThreeColor()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 2, Region::committed ) );
	m1.AddBlock( Region( 2, 10, Region::free ) );

	m2.AddBlock( Region( 0, 3, Region::committed ) );
	m2.AddBlock( Region( 3, 1, Region::free ) );
	m2.AddBlock( Region( 4, 2, Region::committed ) );
	m2.AddBlock( Region( 6, 2, Region::reserved ) );
	m2.AddBlock( Region( 8, 2, Region::committed ) );
	m2.AddBlock( Region( 10, 2, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ExpandRemove()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 4, Region::committed ) );
	m1.AddBlock( Region( 4, 2, Region::reserved ) );
	m1.AddBlock( Region( 6, 2, Region::committed ) );
	m1.AddBlock( Region( 8, 2, Region::reserved ) );
	m1.AddBlock( Region( 10, 2, Region::free ) );

	m2.AddBlock( Region( 0, 9, Region::committed ) );
	m2.AddBlock( Region( 9, 1, Region::reserved ) );
	m2.AddBlock( Region( 10, 2, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void ComplexReverse()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 4, Region::committed ) );
	m1.AddBlock( Region( 4, 2, Region::reserved ) );
	m1.AddBlock( Region( 6, 2, Region::committed ) );
	m1.AddBlock( Region( 8, 2, Region::reserved ) );
	m1.AddBlock( Region( 10, 2, Region::free ) );

	m2.AddBlock( Region( 0, 9, Region::reserved ) );
	m2.AddBlock( Region( 9, 1, Region::committed ) );
	m2.AddBlock( Region( 10, 2, Region::free ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void AddEndAgain()
{
	MemMon::MemoryMap m1, m2;

	m1.AddBlock( Region( 0, 4, Region::committed ) );
	m1.AddBlock( Region( 4, 1, Region::reserved ) );
	m1.AddBlock( Region( 5, 5, Region::committed ) );

	m2.AddBlock( Region( 0, 1, Region::committed ) );
	m2.AddBlock( Region( 1, 1, Region::reserved ) );
	m2.AddBlock( Region( 2, 1, Region::committed ) );
	m2.AddBlock( Region( 3, 2, Region::reserved ) );
	m2.AddBlock( Region( 5, 5, Region::committed ) );

	MemMon::MemoryDiff md( m1, m2 );

	md.Apply( m1 );

	HSHG_ASSERT( m1 == m2 );
}

void MapIoTrip()
{
	MemMon::MemoryMap m1, m2;
	m1.AddBlock( Region( 0, 3, Region::committed ) );
	m1.AddBlock( Region( 3, 1, Region::free ) );
	m1.AddBlock( Region( 4, 2, Region::committed ) );
	m1.AddBlock( Region( 6, 2, Region::reserved ) );
	m1.AddBlock( Region( 8, 2, Region::committed ) );
	m1.AddBlock( Region( 10, 2, Region::free ) );

	m1.Stamp();

	std::stringbuf buf;
	std::streambuf* bufptr = &buf;

	m1.Write( bufptr );
	buf.pubsync();

	m2.Read( bufptr );

	HSHG_ASSERT( m1 == m2 );
}

}

HSHG_BEGIN_TESTS
HSHG_TEST_ENTRY( DiffAddMid )
HSHG_TEST_ENTRY( DiffAddStart )
HSHG_TEST_ENTRY( DiffAddEnd )
HSHG_TEST_ENTRY( PatchAddMid )
HSHG_TEST_ENTRY( PatchAddStart )
HSHG_TEST_ENTRY( PatchAddEnd )
HSHG_TEST_ENTRY( PatchAddSplit )
HSHG_TEST_ENTRY( DiffIoTrip )
HSHG_TEST_ENTRY( MergeAdjacentFree )
HSHG_TEST_ENTRY( MoveBoundary1 )
HSHG_TEST_ENTRY( MoveBoundary2 )
HSHG_TEST_ENTRY( MoveDoubleBoundary )
HSHG_TEST_ENTRY( ExpandBothEnds )
HSHG_TEST_ENTRY( ContractBothEnds )
HSHG_TEST_ENTRY( ContractRemove )
HSHG_TEST_ENTRY( ThreeColor )
HSHG_TEST_ENTRY( ExpandRemove )
HSHG_TEST_ENTRY( ComplexReverse )
HSHG_TEST_ENTRY( AddEndAgain )
HSHG_TEST_ENTRY( MapIoTrip )
HSHG_END_TESTS

HSHG_TEST_MAIN
