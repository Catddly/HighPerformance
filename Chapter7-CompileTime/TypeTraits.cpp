#include <type_traits>
#include <iostream>

// Note: Two categories of type traits:
// 1. Type traits that return information about a type as a boolean or an integer value.
// 2. Type traits that return a new type. These type traits are also called metafunctions.

// A constexpr function has a few restrictions; it is not allowed to do the following:
// 
// 1. Handle local static variables
// 2. Handle thread_local variables
// 3. Call any function, which, in itself, is not a constexpr function

class Planet
{};
class Mars : public Planet
{};
class Sun
{};

static_assert( std::is_base_of_v<Planet, Mars> );
static_assert( !std::is_base_of_v<Planet, Sun> );

template<typename T>
constexpr auto sign_func( T v ) -> int
{
	if constexpr ( std::is_unsigned_v<T> )
	{
		return 1;
	}
	else
	{
		return v < 0 ? -1 : 1;
	}
}

// can be invoked at runtime and compile time
constexpr auto sum( int x, int y, int z )
{
	return x + y + z;
}

// must be evaluated at compile time
// this is a 'immediate function'
consteval auto sum_ct( int x, int y, int z ) // C++20
{
	return x + y + z;
}

struct Bear
{
	auto roar() const
	{
		std::cout << "roar\n";
	}
};

struct Duck
{
	auto quack() const
	{
		std::cout << "quack\n";
	}
};

template <typename T>
constexpr auto make_false()
{
	return false;
}

template <typename Animal>
auto speak( const Animal& a )
{
	// compile-time polymorphism
	if constexpr ( std::is_same_v<Animal, Bear> )
	{
		a.roar();
	}
	else if constexpr ( std::is_same_v<Animal, Duck> )
	{
		a.quack();
	}
	else
	{
		// Trigger compilation error
		static_assert( make_false<Animal>(), "Not an animal!" );
	}
}

template <typename T>
auto generic_mod( const T& v, const T& n ) -> T
{
	assert( n != 0 );
	if constexpr ( std::is_floating_point_v<T> )
	{
		return std::fmod( v, n );
	}
	else
	{
		return v % n;
	}
}

void TypeTraits()
{
	auto same_type = std::is_same_v<uint8_t, unsigned char>;
	auto is_float_or_double = std::is_floating_point_v<decltype( 3.f )>;

	using value_type = std::remove_pointer_t<int*>;  // -> int
	using ptr_type = std::add_pointer_t<float>;      // -> float*

	sign_func( 2 );
	sign_func( -5 );

	// An expression prefixed with the constexpr keyword tells the compiler that the expression should be evaluated at compile time.
	constexpr auto v = 42 + 68 + 5;

	// this can be evaluated at compile time.
	constexpr auto sum0 = sum( 3, 4, 5 );
	// this might be evaluated at compile time (decided by the compiler).
	auto x = 2;
	auto sum1 = sum( x, 5, 6 );

	constexpr auto sum2 = sum_ct( 3, 4, 5 );
	// [ERROR] won't compile
	// auto x = 2;
	// auto sum3 = sum_ct( x, 5, 6 );

	auto bear = Bear{};
	speak( bear );
	auto duck = Duck{};
	speak( duck );
}