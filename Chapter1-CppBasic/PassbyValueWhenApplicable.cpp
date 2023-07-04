#include <string>
#include <vector>
#include <iostream>

// Argument s is a const reference
auto str_to_lower_lvalue( const std::string& s ) -> std::string
{
	std::string clone = s;
	for ( auto& c : clone ) c = std::tolower( c );
	return clone;
}

// Argument s is an rvalue reference
auto str_to_lower_rvalue( std::string&& s ) -> std::string
{
	for ( auto& c : s ) c = std::tolower( c );
	return s;
}

// This can cover both lvalue reference and rvalue reference case!!
auto str_to_lower( std::string s ) -> std::string
{
	for ( auto& c : s ) c = std::tolower( c );
	return s;
}

// But in this case, we better seperate const reference and rvalue reference version
class Widget
{
	std::vector<int> data_{};
	// ...
public:
	//void set_data( std::vector<int> x ) // Not good, we cannot reuse the allocated memory inside data_.
	//{
	//	data_ = std::move( x );
	//}

	// keep overload function
	void set_data( const std::vector<int>& x )
	{
		data_ = x;
	}
	void set_data( std::vector<int>&& x ) noexcept
	{
		data_ = std::move( x );
	}
};

// When initializing class members in a constructor, we can safely use the pass-by-value-then-move pattern.
// During the construction of a new object, there is no chance that there are pre-allocated buffers
// that could have been utilized to avoid heap allocations.

// This is Ok
class Widget1
{
	std::vector<int> data_;
public:
	Widget1( std::vector<int> x )  // By value
		: data_{ std::move( x ) }  // Move-construct
	{}
	// ...
};

void PassbyValueWhenApplicable()
{
	// Copy once, and move out of function str_to_lower()
	auto str0 = std::string{ "ABC" };
	str0 = str_to_lower( str0 );
	std::cout << str0 << "\n";

	// Move first and move out of function str_to_lower() again, no copy happened
	auto str1 = std::string{ "ABC" };
	str1 = str_to_lower( std::move( str1 ) );
	std::cout << str1 << "\n";
}