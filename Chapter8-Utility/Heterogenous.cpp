#include <utility>
#include <iostream>
#include <algorithm>
#include <map>
#include <sstream>
#include <any>
#include <vector>
#include <variant>

template <size_t Index, typename Tuple, typename Func>
constexpr void tuple_at( const Tuple& t, Func f )
{
	const auto& v = std::get<Index>( t );
	std::invoke( f, v );
}

template <typename Tuple, typename Func, size_t Index = 0>
constexpr void tuple_for_each( Tuple const& t, Func f )
{
	constexpr auto TUPLE_SIZE = std::tuple_size_v<Tuple>;
	if constexpr ( Index < TUPLE_SIZE )
	{
		tuple_at<Index>( t, f );
		tuple_for_each<Tuple, Func, Index + 1>( t, f );
	}
}

template <typename Tuple, typename Func, size_t Index = 0>
constexpr auto tuple_any_of( const Tuple& t, const Func& f ) -> bool
{
	constexpr auto n = std::tuple_size_v<Tuple>;
	if constexpr ( Index < n )
	{
		bool success = std::invoke( f, std::get<Index>( t ) );
		if ( success )
		{
			return true;
		}
		return tuple_any_of<Tuple, Func, Index + 1>( t, f );
	}
	else
	{
		return false;
	}
}

// variadic parameter pack
template <typename ...Ts>
auto expand_pack( const Ts& ...values )
{
	auto tuple = std::tie( values... );
}

// template <typename... Ts> expands to template <typename T0, typename T1>
// expand_pack(const Ts& ...values) expands to expand_pack(const T0& v0, const T1& v1)
// std::tie(values...) expands to std::tie(v0, v1)

// variadic parameter pack
template <typename ...Ts>
auto make_string( const Ts& ...values )
{
	auto ss = std::ostringstream{};
	// Create a tuple of the variadic parameter pack 
	auto tuple = std::tie( values... );
	// Iterate the tuple 
	tuple_for_each( tuple, [ &ss ] ( const auto& v )
	{
		ss << v;
	} );
	return ss.str();
}

template<class... Lambdas>
struct Overloaded : Lambdas...
{
	using Lambdas::operator()...;
};

// this is a template deduction guide
template<class... Lambdas>
Overloaded( Lambdas... ) -> Overloaded<Lambdas...>;

// template deduction guide
template<typename Iterator>
void func( Iterator first, Iterator last )
{
	// compiler do _NOT_ know the T in std::vector<T>
	std::vector v( first, last );
}

namespace std
{
	// this is a template deduction guide
	template <typename Iter>
	std::vector( Iter b, Iter e )->std::vector<typename std::iterator_traits<Iter>::value_type>;
}

// reflection
class RPlayer
{
public:
	RPlayer( std::string name, int level, int score )
		: name_{ std::move( name ) }, level_{ level }, score_{ score }
	{}

	// positive reflection implementation
	// this is not the 'strictly speaking' reflection
	auto reflect() const
	{
		return std::tie( name_, level_, score_ );
	}
private:
	std::string name_;
	int level_{};
	int score_{};
};

template <typename T>
concept Reflectable = requires ( T& t )
{
	t.reflect();
};

auto& operator<<( std::ostream& ostr, const Reflectable auto& p )
{
	tuple_for_each( p.reflect(), [ &ostr ] ( const auto& m )
	{
		ostr << m << " ";
	} );
	return ostr;
}

void Heterogenous()
{	
	using namespace std::string_literals;

	std::pair<int, int> v = std::minmax( { 4, 3, 2, 4, 5, 1 } );
	std::cout << v.first << " " << v.second << "\n";

	auto scores = std::map<std::string, int>{};
	scores.insert( std::pair{ "Neo", 12 } ); // Correct but ineffecient
	scores.emplace( "Tri", 45 );             // Use emplace() instead
	scores.emplace( "Ari", 33 );

	for ( auto&& it : scores ) // "it" is a std::pair
	{
		auto& key = it.first;
		auto& val = it.second;
		std::cout << key << ": " << val << '\n';
	}

	std::cout << "\n";

	// using structure binding (modern cpp way)
	for ( auto const& [key, val] : scores )
	{
		std::cout << key << ": " << val << '\n';
	}

	std::cout << "\n";

	//-------------------------------------------------------------------------

	auto t0 = std::tuple<int, std::string, bool>{};
	auto t1 = std::tuple{ 0, std::string{}, false }; // deduced by compiler
	auto t2 = std::make_tuple( 42, std::string{ "hi" }, true );

	// generate a class like this
	//struct Tuple
	//{
	//	int data0_{};
	//	std::string data1_{};
	//	bool data2_{};
	//};

	auto a = std::get<0>( t2 ); // int
	auto b = std::get<1>( t2 ); // std::string
	auto c = std::get<2>( t2 ); // bool

	std::cout << a << "\n";
	std::cout << b << "\n";
	std::cout << c << "\n";

	std::cout << "\n";

	auto func = [] ( auto const& val )
	{
		std::cout << val << "\n";
	};

	tuple_at<0>( t2, func );
	tuple_for_each( t2, func );

	std::cout << "\n";

	auto t = std::tuple{ 42, 43.0f, 44.0 };
	auto has_44 = tuple_any_of( t, [] ( auto v )
	{
		return v == 44;
	} );
	std::cout << std::boolalpha << has_44 << "\n";

	{
		auto make_saturn = [] ()
		{
			return std::tuple{ "Saturn"s, 82, true };
		};

		{
			// this is verbose and tedious
			auto name = std::string{};
			auto n_moons = int{};
			auto rings = bool{};
			std::tie( name, n_moons, rings ) = make_saturn();
			std::cout << name << ' ' << n_moons << ' ' << rings << '\n';
		}

		{
			const auto& [name, n_moons, rings] = make_saturn(); // C++17
			std::cout << name << ' ' << n_moons << ' ' << rings << '\n';
		}

		auto planets = {
			std::tuple{"Mars"s, 2, false},
			std::tuple{"Neptune"s, 14, true}
		};

		for ( auto&& [name, n_moons, rings] : planets )
		{
			std::cout << name << ' ' << n_moons << ' ' << rings << '\n';
		}
	}

	std::cout << "\n";

	auto str = make_string( "C++", 20 );  // OK: str is "C++"
	std::cout << str << "\n";

	// Dynamically sized heterogenous collections
	//-------------------------------------------------------------------------

	std::cout << "\n";

	auto container = std::vector<std::any>{ 42, "hi", true };

	// Drawback to use std::any:
	// 1. Runtime type check due to type erased to ensure type safety.
	// 2. std::any allocates object on the heap.

	for ( const auto& a : container )
	{
		// [WARN] typeid is used!!! which means RTTI is used! Big performance penalty
		if ( a.type() == typeid( int ) )
		{
			const auto& value = std::any_cast<int>( a );
			std::cout << value << "\n";
		}
		else if ( a.type() == typeid( const char* ) )
		{
			const auto& value = std::any_cast<const char*>( a );
			std::cout << value << "\n";
		}
		else if ( a.type() == typeid( bool ) )
		{
			const auto& value = std::any_cast<bool>( a );
			std::cout << value << "\n";
		}
	}

	//-------------------------------------------------------------------------
	
	std::cout << "\n";

	// The std::variant has two main advantages over std::any:
	// 
	// 1. It does not store its contained type on the heap( unlike std::any )
	// 2. It can be invoked with a generic lambda, meaning you don't explicitly have to know its currently contained type (more about this in the later sections of this chapter)

	using VariantType = std::variant<int, std::string, bool>;
	VariantType vari{};

	if ( std::holds_alternative<int>( vari ) ) // true, int is first alternative
	{
		vari = 7;
	}
	if ( std::holds_alternative<int>( vari ) ) // true
	{
		vari = std::string{ "Anne" };
	}
	if ( std::holds_alternative<int>( vari ) ) // false, int was overwritten 
	{
		vari = false;
	}
	if ( std::holds_alternative<bool>( vari ) ) // true, v is now bool
	{
		std::cout << "Now it holds bool!\n";
	}

	struct Widget
	{
		explicit Widget( int )
		{    // Throwing constructor
			throw std::exception{};
		}
	};

	auto var = std::variant<double, Widget>{ 1.0 };
	try
	{
		var.emplace<1>( 42 ); // Try to construct a Widget instance
	}
	catch ( ... )
	{
		std::cout << "exception caught\n";
		if ( var.valueless_by_exception() ) // var may or may not recover
		{
			std::cout << "valueless\n";     // be valueless
		}
		else
		{
			std::cout << std::get<0>( var ) << '\n';
		}
	}

	{
		auto var = std::variant<int, bool, float>{};
		std::visit( [] ( auto&& val )
		{
			std::cout << val << "\n";
		}, var );

		auto overloaded_lambdas = Overloaded {
			[] ( int v ) { std::cout << "Int: " << v << "\n"; },
			[] ( bool v ) { std::cout << "Bool: " << std::boolalpha << v << "\n"; },
			[] ( float v ) { std::cout << "Float: " << v << "\n"; }
		};

		// [ERROR] This will crash in C++17!!

		overloaded_lambdas( 1.24f );
		overloaded_lambdas( false );
		overloaded_lambdas( 745.4f );

		std::visit( overloaded_lambdas, var );
	}

	//-------------------------------------------------------------------------

	using VariantType = std::variant<int, std::string, bool>;
	auto cont = std::vector<VariantType>{};

	cont.push_back( false );
	cont.push_back( "I am a string"s );
	cont.push_back( "I am also a string"s );
	cont.push_back( 13 );

	//-------------------------------------------------------------------------
	
	std::cout << "\n";

	struct Player
	{
		std::string name_{};
		int level_{};
		int score_{};
		// etc...
	};

	auto players = std::vector<Player>{};
	// Add players here...
	players.emplace_back( "timmy", 1, 12 );
	players.emplace_back( "zhang", 2, 43 );
	players.emplace_back( "liu", 2, 23 );
	players.emplace_back( "tommy", 1, 52 );
	players.emplace_back( "jack", 3, 12 );

	auto cmp = [] ( const Player& lhs, const Player& rhs )
	{
		auto p1 = std::tie( lhs.level_, lhs.score_ ); // Projection
		auto p2 = std::tie( lhs.level_, lhs.score_ ); // Projection
		// compare level first, then score
		return p1 < p2;
	};
	std::sort( players.begin(), players.end(), cmp );

	// even cleaner code!
	std::ranges::sort( players, std::less{}, [] ( const Player& p )
	{
		return std::tie( p.level_, p.score_ );
	} );

	for ( auto const& player : players )
	{
		std::cout << player.name_ << "\n";
	}

	std::cout << "\n";

	auto player0 = RPlayer{ "Kai", 4, 2568 };
	std::cout << player0 << "\n";
}