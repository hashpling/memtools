#ifndef MMVALUEPTR_H
#define MMVALUEPTR_H

#include <algorithm>
#include <utility>

namespace MemMon
{

	template< class T >
	struct Copier
	{
		static T* MakeClone( const T* t ) { return new T( *t ); }
	};

	template< class T >
	struct Cloner
	{
		static T* MakeClone( const T* t ) { return t->Clone(); }
	};

	// Class like std::auto_ptr, but copies its target on
	template< class T, template< class > class Cloner >
	class ValuePtr
	{
	public:
		~ValuePtr() throw()
		{
			delete _t;
		}

		explicit ValuePtr( T* t = 0 ) throw() : _t( t ) {}

		ValuePtr( const ValuePtr& r )
			: _t( r._t ? Cloner< T >::MakeClone( r._t ) : 0 )
		{
		}

		ValuePtr& operator=( const ValuePtr& r )
		{
			ValuePtr p( r );
			Swap( p );
			return *this;
		}

		T* get() const throw() { return _t; }
		T* release() throw() { T* t = _t; _t = 0; return t; }

		T* operator->() const throw() { return get(); }

		void reset( T* t ) throw()
		{
			if( t != _t )
			{
				delete _t;
				_t = t;
			}
		}

		void Swap( ValuePtr& r ) throw()
		{
			std::swap( _t, r._t );
		}

	private:
		T* _t;
	};

}

namespace std
{
	template< class T, template< class > class C >
	void swap( MemMon::ValuePtr< T, C >& l, MemMon::ValuePtr< T, C >& r ) { l.Swap( r ); }
}

#endif//MMVALUEPTR_H
