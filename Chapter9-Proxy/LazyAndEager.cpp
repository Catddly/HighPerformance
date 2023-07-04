// Note: Lazy evaluation is a technique used to postpone an operation until its result is really needed.
// The opposite, where operations are performed right away, is called eager evaluation.

#include <string>
#include <functional>
#include <iostream>

struct Image // Buffer with JPG data
{ 
	/* ... */
	std::string		m_uri;
};

auto load( std::string_view path ) -> Image // Load image at path
{
	return Image{ std::string{path} };
}

class ScoreView
{
public:
	// Eager, requires loaded bonus image
	void display( const Image& bonus )
	{

	}

	// Lazy, only load bonus image if necessary
	void display( std::function<Image()> bonus )
	{

	}
	// ...
};

// by comparing this way, we have no temporary string constructed.
// but user interface is more difficult to use.
auto is_concat_equal( const std::string& a, const std::string& b,
					  const std::string& c )
{
	return
		a.size() + b.size() == c.size() &&
		std::equal( a.begin(), a.end(), c.begin() ) &&
		std::equal( b.begin(), b.end(), c.begin() + a.size() );
}

class String
{
public:
	String() = default;
	String( std::string str ) : str_{ std::move( str ) }
	{}
	std::string str_{};
};

// use a proxy object (proxy class)
struct ConcatProxy
{
	const std::string& a;
	const std::string& b;

	// can be easily cast back to String
	operator String() const && // only apply to rvalue!
	{
		return String{ a + b };
	}
};

auto operator+( const String& a, const String& b )
{
	// we do _NOT_ perform std::string concatenation here!
	return ConcatProxy{ a.str_, b.str_ };
}

// by using rvalue reference here, we forcing the user not to create proxy object on the stack by variable.
auto operator==( ConcatProxy&& concat, const String& str )
{
	// postpond the comparison operation to here
	return is_concat_equal( concat.a, concat.b, str.str_ );
}

// for C++17
//auto operator==( const String& str, ConcatProxy&& concat )
//{
//	// postpond the comparison operation to here
//	return is_concat_equal( concat.a, concat.b, str.str_ );
//}

void LazyAndEager()
{
	auto score = ScoreView{};

	// Always load bonus image eagerly
	const auto eager = load( "/images/stars.jpg" );
	score.display( eager );

	// Load default image lazily if needed
	// this lambda is a proxy
	auto lazy = []
	{
		return load( "/images/stars.jpg" );
	};
	score.display( lazy );

	auto a = std::string{ "Cole" };
	auto b = std::string{ "Porter" };
	auto c = std::string{ "ColePorter" };
	// a + b construct a temporary std::string here!
	auto is_equal = ( a + b ) == c;        // true

	if ( is_equal )
	{
		std::cout << "It is equal!\n";
	}

	if ( is_concat_equal( a, b, c ) ) // more efficient when this comparation task is heavy
	{
		std::cout << "It is equal!\n";
	}

	auto a0 = String{ "Cole" };
	auto b0 = String{ "Porter" };
	auto c0 = String{ "ColePorter" };
	// no temporary std::string is created!
	is_equal = ( a0 + b0 ) == c0; // true
	//is_equal = c0 == ( a0 + b0 ); // true (In C++20, the operator is symmetry, but in C++17 it is a compilation error)

	if ( is_equal )
	{
		std::cout << "It is equal!\n";
	}

	auto concat = String{ "Cole" } + String{ "Porter" }; // two String object is destructed after this line
	// By using rvalue in parameter, we save the posibility to crash at runtime or meanless result
	// auto is_cole_porter = ( concat == String{ "ColePorter" } ); (this code will _NOT_ compiled)

	//if ( is_cole_porter )
	//{
	//	std::cout << "It is equal!\n";
	//}

	// this is correct! and user have no idea ConcatProxy is exist.
	String concat_str = String{ "Marc" } + String{ "Chagall" };
	// auto is not useful in this case (auto will be ConcatProxy)
	// auto concat_str0 = String{ "Marc" } + String{ "Chagall" };

	std::cout << "\n";
}