#include <ranges>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>

auto reset( std::span<int> values, int n )
{
	for ( auto& i : std::ranges::take_view{ values, n } )
	{
		i = int{};
	}
}

auto to_vector( auto&& r )
{
	std::vector<std::ranges::range_value_t<decltype( r )>> v;
	if constexpr ( std::ranges::sized_range<decltype( r )> )
	{
		v.reserve( std::ranges::size( r ) );
	}
	std::ranges::copy( r, std::back_inserter( v ) );
	return v;
}

auto split( std::string_view s, char delim )
{
	const auto to_string = [] ( auto&& r ) -> std::string
	{
		auto const citer = std::counted_iterator{ r.begin(), std::ssize( r ) };
		auto const cv = std::ranges::common_view{ std::ranges::subrange{ citer, std::default_sentinel } };
		return { cv.begin(), cv.end() }; // after returned, a std::string will be constructed using std::string(begin_iterator, end_iterator)
	};

	return to_vector( std::ranges::split_view{ s, delim }
		| std::views::transform( to_string ) );
}

void ViewsBefore()
{
	// generating views
	//-------------------------------------------------------------------------
	for ( auto value : std::views::iota( -2, 2 ) )
	{
		std::cout << value << " ";
	}
	std::cout << '\n';

	std::cout << '\n';

	// transforming views
	//-------------------------------------------------------------------------

	auto csv = std::string{ "10,11,12" };
	auto digits = csv
		| std::views::split( ',' ) // [ [1, 0], [1, 1], [1, 2] ]
		| std::views::join;        // [ 1, 0, 1, 1, 1, 2 ]

	for ( auto i : digits )
	{
		std::cout << i;
	}

	std::cout << '\n';

	// sampling views
	//-------------------------------------------------------------------------

	auto vec = std::vector{ 1, 2, 3, 4, 5, 4, 3, 2, 1 };
	auto view = vec 
		| std::views::drop_while( [] ( auto i )
	{
		return i < 5;
	} )
		| std::views::take( 3 );

	for ( auto i : view )
	{
		std::cout << i << " ";
	}
	std::cout << '\n';

	std::cout << '\n';

	// utility views
	//-------------------------------------------------------------------------

	auto ifs = std::ifstream( "numbers.txt" );
	for ( auto f : std::ranges::istream_view<float>( ifs ) )
	{
		std::cout << f << '\n';
	}
	ifs.close();

	std::cout << '\n';

	// string_view
	// There is no guarantee that these views can be constructed in constant time!!
	// For example, constructing a std::string_view from a null-terminated C-style string could invoke a call to strlen(), which is an O(n) operation.
	//-------------------------------------------------------------------------

	const char* c_str = "ABC,DEF,GHI";    // C style string
	const auto v1 = split( c_str, ',' );  // std::vector<std::string>
	const auto s = std::string{ "ABC,DEF,GHI" };
	const auto v2 = split( s, ',' );      // std::vector<std::string>
	assert( v1 == v2 );                   // true

	// span
	//-------------------------------------------------------------------------

	int a[]{ 33, 44, 55, 66, 77 };
	reset( a, 3 );

	for ( auto i : a )
	{
		std::cout << i << " ";
	}
	std::cout << '\n';

	auto v = std::vector{ 33, 44, 55, 66, 77 };
	reset( v, 2 );

	for ( auto i : v )
	{
		std::cout << i << " ";
	}
	std::cout << '\n';

	std::cout << '\n';
}