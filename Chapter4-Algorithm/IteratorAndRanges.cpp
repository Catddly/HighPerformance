#include <ranges>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>

// Iterator's respondsibility:
// 
// 1. Navigating in the sequence.
// 2. Reading and writing the value at its current position.
//

// Iterator operations:
//
// std::next(it) or ++
// std::prev(it) or --
// std::advance(it, n) or it += n

// Iterator categories:    
// 
// input / output             |
// forward                    |
// bidirectional              |
// random_access              |
// contiguous                \/

template<class T>
concept my_range = requires( T& t )
{
	std::ranges::begin( t );
	std::ranges::end( t );
};

// A range with identical iterator and sentinel types fulfills the concept of std::ranges::common_range.

struct Flower
{
	// operator generated for us
	auto operator<=>( const Flower& f ) const = default;
	bool operator==( const Flower& ) const = default;
	int height_{};
};

struct Player
{
	std::string name_{};
	int level_{};
	float health_{};
};

// make it generic!
template <typename Iterator, typename T>
auto contains( Iterator begin, Iterator end, const T& v )
{
	for ( auto it = begin; it != end; ++it )
	{
		if ( *it == v )
		{
			return true;
		}
	}
	return false;
}

// add concept constrain!
auto contains( const std::ranges::range auto& r, const auto& x )
{
	auto it = std::begin( r );
	auto sentinel = std::end( r );
	return contains( it, sentinel, x );
}
// same as:
//template <std::ranges::range Iter>
//auto contains( const Iter r, const auto& x )
//{
//	auto it = std::begin( r );
//	auto sentinel = std::end( r );
//	return contains( it, sentinel, x );
//}

// Create data structures which can be used by generic algorithm
struct Grid
{
	Grid( std::size_t w, std::size_t h ) : w_{ w }, h_{ h }
	{
		data_.resize( w * h );
	}
	auto get_row( std::size_t y ); // Returns iterators or a range

	std::vector<int> data_{};
	std::size_t w_{};
	std::size_t h_{};
};

auto Grid::get_row( std::size_t y )
{
	//auto first = data_.begin() + w_ * y;
	//auto sentinel = first + w_;
	//return std::ranges::subrange{ first, sentinel };
	
	// even lazier!
	auto first = data_.begin() + w_ * y;
	return std::views::counted( first, w_ );
	// counted is an inline constexpr functor inside namespace views
}

void IteratorAndRanges()
{
	// since vector contain begin() and end() function, it can be used by ranges.
	auto vec = std::vector{ 1, 1, 0, 1, 1, 0, 0, 1 };
	std::cout << std::ranges::count( vec, 0 ) << "\n";

	// Algorithms do not change the size of the container
	// The fact that standard library algorithms never change the size of a container means
	// that we need to allocate data ourselves when calling algorithms that produce output.

	auto v = std::vector{ 1,1,2,2,3,3 };
	auto new_end = std::remove(
		v.begin(), v.end(), 2
	);
	std::cout << v.size() << '\n';
	// size did NOT change
	v.erase( new_end, v.end() );
	std::cout << v.size() << '\n';
	// size did changed

	v = std::vector{ 1,1,2,2,3,3 };
	new_end = std::unique(
		v.begin(), v.end() );
	std::cout << v.size() << '\n';
	// size did NOT change
	v.erase( new_end, v.end() );
	std::cout << v.size() << '\n';

	// [ERROR] crash! we need to allocate memory for squared!
	//const auto square_func = [] ( int x )
	//{
	//	return x * x;
	//};
	//v = std::vector{ 1, 2, 3, 4 };
	//auto squared = std::vector<int>{};
	//std::ranges::transform( v, squared.begin(), square_func );

	// correct! we preallocate enough size for vector.
	const auto square_func = [] ( int x )
	{
		return x * x;
	};
	v = std::vector{ 1, 2, 3, 4 };
	auto squared = std::vector<int>( v.size() );
	std::ranges::transform( v, squared.begin(), square_func );

	// correct! Use insert allocator.
	v = std::vector{ 1, 2, 3, 4 };
	// Insert into back of vector using std::back_inserter
	auto squared_vec = std::vector<int>{};
	auto dst_vec = std::back_inserter( squared_vec );
	std::ranges::transform( v, dst_vec, square_func );

	std::ranges::for_each( squared_vec, [] ( auto const& val )
	{
		std::cout << val << " ";
	} );
	std::cout << '\n';

	// Insert into a std::set using std::inserter
	auto squared_set = std::set<int>{};
	auto dst_set = std::inserter( squared_set, squared_set.end() );
	std::ranges::transform( v, dst_set, square_func );

	std::ranges::for_each( squared_vec, [] ( auto const& val )
	{
		std::cout << val << " ";
	} );
	std::cout << '\n';

	auto names = std::vector<std::string>{
		"Ralph", "Lisa", "Homer", "Maggie", "Apu", "Bart"
	};
	// use projections (we actually sorting by the size)
	std::ranges::sort( names, std::less<>{}, &std::string::size );
	// names is now "Apu", "Lisa", "Bart", "Ralph", "Homer", "Maggie"

	// Find names with length 3
	auto x = std::ranges::find( names, 3, &std::string::size );
	// x points to "Apu"

	auto players = std::vector<Player>{
		{"Aki", 1, 9.f},
		{"Nao", 2, 7.f},
		{"Rei", 2, 3.f} 
	};
	auto level_and_health = [] ( const Player& p )
	{
		return std::tie( p.level_, p.health_ );
	};
	// Order players by level, then health
	std::ranges::sort( players, std::greater<>{}, level_and_health );

	v = std::vector{ 1, 2, 3, 4 };
	if ( contains( v, 3 ) )
	{
		std::cout << "v contain 3!\n";
	}

	// Custom data structures with generic algorithm
	//-------------------------------------------------------------------------

	auto grid = Grid{ 10, 10 };
	auto y = 3;
	auto row = grid.get_row( y );
	std::ranges::generate( row, std::rand );
	auto num_fives = std::ranges::count( row, 5 );
}