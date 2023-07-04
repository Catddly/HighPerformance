#include <memory>
#include <iostream>
#include <cassert>
#include <stdlib.h>

// The CPU reads memory into its registers one word at a time.
// The word size is 64 bits on a 64 - bit architecture, 32 bits on a 32 - bit architecture.

struct User
{
	char const*			m_name{};
	char const*         m_year{};
	std::uint8_t		m_age{};
	bool				m_hasPassword{};

	// class specified overload new and delete
	//-------------------------------------------------------------------------
	auto operator new( size_t size ) -> void*
	{
		std::cout << "Allocate happen for User class!\n";
		return ::operator new( size );
	}
	auto operator delete( void* p ) -> void
	{
		std::cout << "Deallocate happen for User class!\n";
		::operator delete( p );
	}
};


// Global overload new and delete
//-------------------------------------------------------------------------

auto operator new( size_t size ) -> void*
{
	void* p = std::malloc( size );
	std::cout << "allocated " << size << " byte(s)\n";
	return p;
}

// C++17 alignment version
void* operator new( std::size_t size, std::align_val_t al )
{
	void* p = _aligned_malloc( size, static_cast<size_t>( al ) );
	std::cout << "alignment allocated " << size << " byte(s)\n";
	return p;
}

auto operator delete( void* p ) noexcept -> void
{
	std::cout << "deleted memory\n";
	return std::free( p );
}

void operator delete  ( void* ptr, std::align_val_t al ) noexcept
{
	std::cout << "alignment deleted memory\n";
	return _aligned_free( ptr );
}

auto operator new[] ( size_t size ) -> void*
{
	void* p = std::malloc( size );
	std::cout << "allocated " << size << " byte(s) with new[]\n";
	return p;
}

auto operator delete[] ( void* p ) noexcept -> void
{
	std::cout << "deleted memory with delete[]\n";
	return std::free( p );
}

//-------------------------------------------------------------------------

bool is_aligned( void* ptr, std::size_t alignment )
{
	assert( ptr != nullptr );
	assert( std::has_single_bit( alignment ) ); // Power of 2 (C++20)

	auto s = std::numeric_limits<std::size_t>::max();
	auto aligned_ptr = ptr;
	std::align( alignment, 1, aligned_ptr, s );
	return ptr == aligned_ptr;
}

struct alignas( 64 ) CacheLine
{
	std::byte data[64];
};

void MemoryUsage()
{
	auto* pMemory = std::malloc( sizeof( User ) );
	// placemement new
	auto* pUser = ::new ( pMemory ) User{};

	pUser->m_name = "Timmy";
	std::cout << pUser->m_name << "\n";

	// manually call dtor
	pUser->~User();
	std::free( pMemory );

	// after C++17, we don't have to use placement new
	pMemory = std::malloc( sizeof( User ) );
	pUser = reinterpret_cast<User*>( pMemory );
	if ( pUser )
	{
		//std::uninitialized_fill_n( pUser, 1, User{ "john" } ); // call ctor
		std::construct_at( pUser, User{ "john" } );// C++20
		std::cout << pUser->m_name << "\n";
		std::destroy_at( pUser ); // call dtor
	}
	std::free( pMemory );

	// use new and delete
	auto* p = new char{ 'a' };
	delete p;
	auto* pu = new User{ "mike" };
	delete pu;

	std::cout << "\n";

	// alignment
	// access aligned memory is for compyter efficiency!
	//-------------------------------------------------------------------------
	std::cout << alignof( int ) << '\n';
	auto pi = new int{};
	assert( is_aligned( pi, 4ul ) );

	auto max_alignment = alignof( std::max_align_t );
	std::cout << "Max alignment: " << max_alignment << "\n";
	assert( is_aligned( pi, max_alignment ) );
	delete pi;

	//-------------------------------------------------------------------------

	auto c1 = char{ 'a' };
	auto c2 = char{ 'b' };

	std::cout << std::distance( &c1, &c2 ) << '\n';
	auto val = alignof( std::max_align_t ) % std::distance( &c1, &c2 );

	// their memory location must be align to std::max_align_t
	assert( std::distance( &c1, &c2 ) % alignof( std::max_align_t ) == 0 );
	
	std::cout << "\n";

	// we can ensure stricter alignment
	//-------------------------------------------------------------------------

	// force two int to be on separate cache line (64 bytes)
	alignas( 64 ) int x{};
	alignas( 64 ) int y{};

	assert( is_aligned( &x, 64 ) );
	assert( is_aligned( &y, 64 ) );

	std::cout << "\n";

	//-------------------------------------------------------------------------

	constexpr auto ps = std::size_t{ 4096 };      // Page size
	struct alignas( ps ) Page
	{
		std::byte data_[ps];
	};

	auto* page = new Page{};          // Memory page
	assert( is_aligned( page, ps ) ); // True
	// Use page ...
	delete page;

	std::cout << "\n";
}