#include <vector>
#include <iostream>
#include <unordered_set>

// Three main categories of container:
// 1. Sequence Container.
// 2. Associative Container.
//    2.1 Ordered Associative Container. These containers are based on trees (self-balancing binary search tree). 
//        They require that the elements are ordered by the less than operator (<).
//    2.2 Unordered Associative Container. These containers are based on hash tables. 
//        They require that the elements are compared with the equality operator (==) and that there is a way to compute a hash value based on an element.
// 3. Container Adaptor.
//    There are three container adaptors in the standard library : std::stack, std::queue, and std::priority_queue.
//    A priority queue is a partially ordered data structure.


// input looks like an array, but is in fact a pointer 
auto f( const int input[] )
{
	// An array losing its size information is usually referred to as array decay.
	// ... 
}

// copy from <boost/functional/hash.hpp>
template <typename T>
auto hash_combine( size_t& seed, T const& v )
{
	seed ^= std::hash<T>{}(v) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

struct Person
{
	std::string name;
	uint8_t     age;
};

void Container()
{
	auto v = std::vector{ -1, 5, 2, -3, 4, -5, 5 };
	std::erase( v, 5 ); // v: [-1,2,-3,4,-5]
	std::erase_if( v, [] ( auto x )
	{
		return x < 0;
	} ); // v: [2, 4]

	// can take in different size of arrays
	int a[16] = {};
	int b[1024] = {};
	f( a ); // Compiles, but unsafe

	auto person_eq = [] ( const Person& lhs, const Person& rhs )
	{
		return lhs.name == rhs.name && lhs.age == rhs.age;
	};
	auto person_hash = [] ( const Person& person )
	{
		auto seed = size_t{ 0 };
		hash_combine( seed, person.name );
		hash_combine( seed, person.age );
		return seed;
	};

	using PersonSet = std::unordered_set<Person, decltype( person_hash ), decltype( person_eq )>;
	auto persons = PersonSet{ 4, person_hash, person_eq };

	persons.insert( { "tommy", 4 } );
	persons.insert( { "jimmy", 44 } );

	if ( persons.contains( Person{ "jimmy", 44 } ) ) // C++20
	{
		std::cout << "Found jimmy!\n";
	}

	std::cout << "Bucket Count: " << persons.bucket_count() << '\n';
	std::cout << "Load Factor: " << persons.load_factor() << '\n';
	std::cout << "Max Load Factor: " << persons.max_load_factor() << '\n';

	// manually rehash all elements
	// it is better to be a prime number!!
	persons.rehash( 13 );
}