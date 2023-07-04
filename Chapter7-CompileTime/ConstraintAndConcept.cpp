#include <cmath>
#include <iostream>
#include <cassert>

// No constraint version
template <typename T>
class Point2D
{
public:
	Point2D( T x, T y ) : x_{ x }, y_{ y }
	{}
	auto x()
	{
		return x_;
	}
	auto y()
	{
		return y_;
	}
	// ...
private:
	T x_{};
	T y_{};
};

// this function depends on only public interface, so it is good to design a separate function instead of a member function.
auto dist( auto p1, auto p2 ) // because the type of p1 and p2 is totolly unconstrained, the return type is also hard to know.
{
	auto a = p1.x() - p2.x();
	auto b = p1.y() - p2.y();
	return std::sqrt( a * a + b * b );
}

// constraints over concept can produce error at instantiation phase!!
template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept Number = FloatingPoint<T> or std::is_integral_v<T>;

template <typename T> requires std::integral<T>
auto mod( T v, T n )
{
	return v % n;
}

template <typename T> requires std::integral<T>
struct Foo
{
	T value;
};

// alternative way
template <std::integral T>
auto mod( T v, T n )
{
	return v % n;
}

// add constraints to abbreviated function parameters
// return type can also be constrained.
std::integral auto mod( std::integral auto v, std::integral auto n )
{
	return v % n;
}

// recall from TypeTraits.cpp generic_mod()
//template <typename T>
//auto generic_mod( T v, T n ) -> T
//{
//	if constexpr ( std::is_floating_point_v<T> )
//	{
//		return std::fmod( v, n );
//	}
//	else
//	{
//		return v % n;
//	}
//}

// this is much clear than 'if constexpr' verions using function overload
template <std::integral T>
auto generic_mod( T v, T n ) -> T // Integral version
{             
	return v % n;
}

template <std::floating_point T>
auto generic_mod( T v, T n ) -> T // Floating point version 
{             
	return std::fmod( v, n );
}

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Point = requires( T p )
{
	// this also implies p must have x() and y() as member function
	requires std::is_same_v<decltype( p.x() ), decltype( p.y() )>; // require the type of the return value of x() and y() must be the same.
	requires Arithmetic<decltype( p.x() )>; // require the return type of x() must be a arithmeric type.
};

// constrained version
std::floating_point auto c_dist( Point auto p1, Point auto p2 )
{
	auto a = p1.x() - p2.x();
	auto b = p1.y() - p2.y();
	return std::sqrt( a * a + b * b );
}

// this is better
template <Arithmetic T>
class CPoint2D
{
public:
	CPoint2D( T x, T y ) : x_{ x }, y_{ y }
	{}
	auto x()
	{
		return x_;
	}
	auto y()
	{
		return y_;
	}
	// ...
private:
	T x_{};
	T y_{};
};

template <typename T>
constexpr auto make_false()
{
	return false;
}

template <typename Dst, typename Src>
auto safe_cast( const Src& v ) -> Dst
{
	using namespace std;
	constexpr auto is_same_type = is_same_v<Src, Dst>;
	constexpr auto is_pointer_to_pointer = is_pointer_v<Src> && is_pointer_v<Dst>;
	constexpr auto is_float_to_float = is_floating_point_v<Src> && is_floating_point_v<Dst>;
	constexpr auto is_number_to_number = is_arithmetic_v<Src> && is_arithmetic_v<Dst>;

	constexpr auto is_intptr_to_ptr =
		( is_same_v<uintptr_t, Src> || is_same_v<intptr_t, Src> )
		&& is_pointer_v<Dst>;

	constexpr auto is_ptr_to_intptr =
		is_pointer_v<Src> &&
		( is_same_v<uintptr_t, Dst> || is_same_v<intptr_t, Dst> );

	if constexpr ( is_same_type )
	{
		return v;
	}
	else if constexpr ( is_intptr_to_ptr || is_ptr_to_intptr )
	{
		return reinterpret_cast<Dst>( v );
	}
	else if constexpr ( is_pointer_to_pointer )
	{
		assert( dynamic_cast<Dst>( v ) != nullptr );
		return static_cast<Dst>( v );
	}
	else if constexpr ( is_float_to_float )
	{
		auto casted = static_cast<Dst>( v );
		auto casted_back = static_cast<Src>( v );
		assert( !isnan( casted_back ) && !isinf( casted_back ) );
		return casted;
	}
	else if constexpr ( is_number_to_number )
	{
		auto casted = static_cast<Dst>( v );
		auto casted_back = static_cast<Src>( casted );
		assert( casted == casted_back );
		return casted;
	}
	else
	{
		// we can _NOT_ use this form of static_assert(), it will always stop compiler to compile
		// static_assert( false, "CastError" );
		// instead, we utilize the template function make_false<T>() to delay the generation until required.
		static_assert( make_false<Src>(), "CastError" ); // this line now will not be generated until make_false is evaluated.
		return Dst{}; // This can never happen, the static_assert should have failed
	}
}

// compile time hash function
constexpr auto hash_function( const char* str ) -> size_t
{
	// do _NOT_ do this in real application
	// this is a bad hash function
	auto sum = size_t{ 0 };
	for ( auto ptr = str; *ptr != '\0'; ++ptr )
		sum += *ptr;
	return sum;
}

class PrehashedString
{
public:
	template <size_t N>
	constexpr PrehashedString( const char( &str )[N] )
		: hash_{ hash_function( &str[0] ) }, size_{ N - 1 },
		// The subtraction is to avoid null at end
		strptr_{ &str[0] }
	{}
	auto operator==( const PrehashedString& s ) const
	{
		return size_ == s.size_ &&
			std::equal( c_str(), c_str() + size_, s.c_str() );
	}
	auto operator!=( const PrehashedString& s ) const
	{
		return !( *this == s );
	}
	constexpr auto size() const
	{
		return size_;
	}
	constexpr auto get_hash() const
	{
		return hash_;
	}
	constexpr auto c_str() const -> const char*
	{
		return strptr_;
	}
private:
	size_t hash_{};
	size_t size_{};
	const char* strptr_{ nullptr };
};


namespace std
{
	template <>
	struct hash<PrehashedString>
	{
		constexpr auto operator()( const PrehashedString& s ) const
		{
			return s.get_hash();
		}
	};
}

auto test_prehashed_string()
{
	const auto& hash_fn = std::hash<PrehashedString>{};
	const auto& str = PrehashedString( "abc" );
	return hash_fn( str );
}

void ConstraintAndConcept()
{
	auto p1 = Point2D{ 2, 2 };
	auto p2 = Point2D{ 6, 5 };
	auto d = dist( p1, p2 );

	std::cout << d << '\n';

	// auto d1 = dist( 3, 4 ); // [ERROR] this error appear after the code generation phase.

	auto from = Point2D{ "2.0", "2.0" }; // Ouch!
	auto to = Point2D{ "6.0", "5.0" };   // Point2D<const char*>
	auto d1 = dist( from, to );
	std::cout << d1 << '\n'; // completely mess!

	// [ERROR] this error appear at the instantiation phase. it clearly points out the argument type doesn't fullfill the requirements of concept Point.
	// auto dd1 = c_dist( 3, 4 );

	//auto from1 = Point2D{ "2.0", "2.0" }; // Ouch!
	//auto to1 = Point2D{ "6.0", "5.0" };   // Point2D<const char*>
	//auto d11 = c_dist( from, to ); // [ERROR] clearly points out concept Arithmetic is evaluated to false!
	//std::cout << d11 << '\n';

	// auto from1 = CPoint2D{ "2.0", "2.0" }; // [ERROR] constrained version of Point2D

	Point auto p3 = CPoint2D{ 2, 2 };
	Point auto p4 = CPoint2D{ 6, 5 };
	auto dd = c_dist( p3, p4 );

	std::cout << dd << '\n';

	// more concepts!
	// std::regular
	// std::semiregular
	// std::equality_comparable
	// std::copyable
	// std::movable

	auto a = 22.6844550;
	auto b = safe_cast<int>( a );
	auto c = safe_cast<int>( 42.0f );

	constexpr auto hash = hash_function( "abc" );

	constexpr auto prehash0 = PrehashedString( "abcd" );
	constexpr auto prehash1 = PrehashedString( "dfjsjs" );

	// [ERROR] can _NOT_ compile
	//auto str = std::string{ "my_string" };
	//auto prehashed_string = PrehashedString{ str.c_str() };

	// [ERROR] can _NOT_ compile
	//auto* strptr = new char[5];
	//auto prehashed_string = PrehashedString{ strptr };

	std::cout << prehash0.get_hash() << "\n";
	std::cout << prehash1.get_hash() << "\n";

	// this only yield the prehash value
	test_prehashed_string();
}