#include <algorithm>
#include <iostream>
#include <vector>
#include <numeric>
#include <cassert>
#include <list>

void print( auto&& r )
{
	std::ranges::for_each( r, [] ( auto&& i )
	{
		std::cout << i << ' ';
	} );
	std::cout << '\n';
}

void Algorithm()
{
	// transform
	//-------------------------------------------------------------------------

	auto in = std::vector{ 1, 2, 3, 4 };
	auto out = std::vector<int>( in.size() );
	auto lambda = [] ( auto&& i )
	{
		return i * i;
	};
	std::ranges::transform( in, out.begin(), lambda );
	print( out );

	std::cout << "\n";

	// generation
	//-------------------------------------------------------------------------

	auto v = std::vector<int>( 4 );
	std::ranges::fill( v, -1 );
	print( v );

	v = std::vector<int>( 4 );
	std::ranges::generate( v, std::rand );
	print( v );

	v = std::vector<int>( 6 );
	// generate number in increasing order
	std::iota( v.begin(), v.end(), 0 );
	print( v );

	std::cout << "\n";

	// sort
	//-------------------------------------------------------------------------

	v = std::vector{ 4, 3, 2, 3, 6 };
	std::ranges::sort( v );
	print( v );

	std::cout << "\n";

	// finding
	//-------------------------------------------------------------------------

	auto col = std::vector{ 2, 4, 3, 2, 3, 1 };
	auto it = std::ranges::find( col, 2 );
	if ( it != col.end() )
	{
		std::cout << *it << '\n';
	}

	v = std::vector{ 2, 2, 3, 3, 3, 4, 5 }; // Sorted!
	assert( std::ranges::is_sorted( v ) );
	bool found = std::ranges::binary_search( v, 3 );
	std::cout << std::boolalpha << found << '\n';

	v = std::vector{ 2, 2, 3, 3, 3, 4, 5 };
	// find the first 3
	it = std::ranges::lower_bound( v, 3 );
	if ( it != v.end() )
	{
		auto index = std::distance( v.begin(), it );
		std::cout << index << '\n';
	}
	// want both upper bound and lower bound
	auto subrange = std::ranges::equal_range( v, 3 );
	if ( subrange.begin() != subrange.end() )
	{
		auto pos1 = std::distance( v.begin(), subrange.begin() );
		auto pos2 = std::distance( v.begin(), subrange.end() );
		std::cout << pos1 << " " << pos2 << '\n';
	}

	std::cout << "\n";

	// testing
	//-------------------------------------------------------------------------

	v = std::vector{ 3, 2, 2, 1, 0, 2, 1 };
	const auto is_negative = [] ( int i )
	{
		return i < 0;
	};

	if ( std::ranges::none_of( v, is_negative ) )
	{
		std::cout << "Contains only natural numbers\n";
	}

	if ( std::ranges::all_of( v, is_negative ) )
	{
		std::cout << "Contains only negative numbers\n";
	}

	if ( std::ranges::any_of( v, is_negative ) )
	{
		std::cout << "Contains at least one negative number\n";
	}

	std::cout << "\n";

	// counting
	//-------------------------------------------------------------------------

	const auto numbers = std::list{ 3, 3, 2, 1, 3, 1, 3 };
	auto n = std::ranges::count( numbers, 3 );
	std::cout << n << '\n';

	v = std::vector{ 0, 2, 2, 3, 3, 4, 5 };
	assert( std::ranges::is_sorted( v ) ); // O(n), but not called in release
	// we know that it is sorted, so equal_range can be used to count!
	// std::count will take O(n) but std::equal_range will only take O(logn)
	auto r = std::ranges::equal_range( v, 3 );
	n = std::ranges::size( r );
	std::cout << n << '\n';

	std::cout << "\n";

	// clamping
	//-------------------------------------------------------------------------

	v = std::vector{ 4, 2, 1, 7, 3, 1, 5 };
	const auto [min, max] = std::ranges::minmax( v );
	std::cout << min << " " << max << '\n';

	v = std::vector{ 4, 2, 7, 1, 1, 3 };
	it = std::ranges::min_element( v );
	std::cout << std::distance( v.begin(), it ) << '\n';
}