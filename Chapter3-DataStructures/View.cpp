#include <string>
#include <span>

// Note: The complexity of substr() using a std::string_view is constant,
// which should be compared to the substr() version of std::string, which runs in linear time.

auto need_copy( const std::string& s )
{
	// process s ...
}

auto need_no_copy( std::string_view s ) // Pass by value
{
	// s.substr(); // This is more efficient than std::string.substr()
	// process s ... 
}

//// buffer looks like an array, but is in fact a pointer 
//auto f1( float buffer[] )
//{
//	const auto n = std::size( buffer );   // Does not compile!
//	for ( auto i = 0u; i < n; ++i )
//	{     // Size is lost!
//// ...
//	}
//}

// Much safer!
auto f3( std::span<float> buffer ) // Pass by value
{
	for ( auto&& b : buffer ) // Range-based for-loop
	{         
// ...
	}
}

void View()
{
	need_copy( "A string literal" ); // Creates a std::string
	need_no_copy( "A string literal" );

	float data0[256] = {};
	float data1[1024] = {};
	f3( data0 );
	f3( data1 );
}