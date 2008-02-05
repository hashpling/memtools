#ifndef MMINFO_H
#define MMINFO_H

#include <streambuf>
#include <vector>
#include <utility>

namespace MemMon
{

struct FreeRegion
{
	size_t base;
	size_t size;
};

struct Region : public FreeRegion
{
	int type;
};

class MemoryMap
{
public:
	template<typename charT, typename traits>
	void Write(std::basic_streambuf<charT, traits>*) const;

	template<typename charT, typename traits>
	void Read(std::basic_streambuf<charT, traits>*);

	std::vector<Region> blocklist;
	std::vector<FreeRegion> freelist;

	size_t total_free;
	size_t total_commit;
	size_t total_reserve;
};
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<( std::basic_ostream<charT, traits>& os,
											   const MemoryMap& mem)
{
	mem.Write(os.rdbuf());
	return os;
}

template<typename charT, typename traitsT>
inline std::basic_istream<charT, traitsT>& operator>>( std::basic_istream<charT, traitsT>& is,
											   MemoryMap& mem)
{
	mem.Read(is.rdbuf());
	return is;
}

inline bool operator==(const Region& lhs, const Region& rhs)
{
	return lhs.base == rhs.base && lhs.size == rhs.size && lhs.type == rhs.type;
}

inline bool operator==(const FreeRegion& lhs, const FreeRegion& rhs)
{
	return lhs.size == rhs.size && (lhs.size == 0 || lhs.base == rhs.base);
}

inline bool operator==(const MemoryMap& lhs, const MemoryMap& rhs)
{
	return lhs.blocklist == rhs.blocklist && lhs.freelist == rhs.freelist;
}

class MemoryDiff
{
public:
	MemoryDiff( const MemoryMap& before, const MemoryMap& after );
	void Apply( MemoryMap& target ) const;
	void ReverseApply( MemoryMap& target ) const;

	template< typename charT, typename traits >
	void Write( std::basic_streambuf< charT, traits >* ) const;

	template<typename charT, typename traits>
	void Read( std::basic_streambuf< charT, traits >*);

private:
	std::vector< Region > additions;
	std::vector< std::pair< size_t, std::pair< int, int > > > changes;
	std::vector< Region > removals;
};

}

#endif//MMINFO_H
