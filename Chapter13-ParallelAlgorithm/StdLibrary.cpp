#include <string>
#include <vector>
#include <algorithm>
#include <execution>
#include <iostream>
#include <ranges>

void StdLibrary()
{
	auto v = std::vector<std::string>{
		"woody", "steely", "loopy", "upside_down"
	};

	// Parallel sort
	std::sort( std::execution::par, v.begin(), v.end() );

	// std::execution::unsequenced
	// The unsequenced policy was added in C++20.
	// It tells the algorithm that the loop is allowed to be vectorized using, for example, SIMD instructions.
	// In practice, this means that you cannot
	// use any synchronization primitives in the code you pass to the algorithm, since this could result in deadlocks.

	//auto vec = std::vector<std::string>{ "Ada", "APL" };
	//auto tot_size = size_t{ 0 };
	//auto mut = std::mutex{};

	// may cause deadlock!
	//std::for_each( std::execution::unseq, vec.begin(), vec.end(),
	//			   [ & ] ( const auto& s )
	//{
	//	auto lock = std::scoped_lock{ mut }; // Lock
	//	tot_size += s.size(); // Unlock
	//} );

	//{ // Iteration 1
	//	const auto& s = *it++;
	//	mut.lock();
	//	tot_size += s.size();
	//	mut.unlock();
	//}
	//{ // Iteration 2
	//	const auto& s = *it++;
	//	mut.lock();
	//	tot_size += s.size();
	//	mut.unlock();
	//}

	// by using std::execution::unseq, this may optimize by compiler to:

	//{ // Iteration 1 & 2 merged
	//	const auto& s1 = *it++;
	//	const auto& s2 = *it++;
	//	mut.lock();
	//	mut.lock();                // Deadlock!
	//	tot_size += s1.size();     // Replace these operations
	//	tot_size += s2.size();     // with vectorized instructions
	//	mut.unlock();
	//	mut.unlock();
	//}

	// std::execution::par_unseq
	// Executes the algorithm in parallel like the parallel policy, with the addition that it may also vectorize the loop.

	//auto v = { 1, 2, 3, 4 };
	//auto f = [] ( auto )
	//{
	//	throw std::exception{};
	//};
	// 
	// // OK! always propagate to caller!
	//try
	//{
	//	std::for_each( v.begin(), v.end(), f );
	//}
	//catch ( ... )
	//{
	//	std::cout << "Exception caught\n";
	//}

	//try
	//{
	//	std::for_each( std::execution::seq, v.begin(), v.end(), f );
	//}
	//catch ( ... )
	//{
	//	// The thrown std::exception never reaches us.
	//	// Instead, std::terminate() has been called 
	//}

	//-------------------------------------------------------------------------

	// std::accumulate() guarantee to accumulate elements in the order of elements.
	// so it can _NOT_ be parallel.
	// Instead, you can use std::reduce() to accumlate, but it doesn't guarantee to be sequential.
	// An interesting point related to performance is that floating-point math is not commutative.

	auto v0 = std::vector<std::string>{ "Ada","Bash","C++" };
	auto num_chars = std::transform_reduce(
		v0.begin(), v0.end(), size_t{ 0 },
		// Reduce	
		[]( size_t a, size_t b )
		{
			return a + b;
		},
		// Transform
			[]( const std::string& s )
		{
			return s.size();
		} );

	std::cout << "Num chars: " << num_chars << '\n';

	//-------------------------------------------------------------------------

	// rare usecase

	struct Func
	{
		void operator()( const std::string& s )
		{
			res_ += s;
		};
		std::string res_{};
	};
	auto v1 = std::vector<std::string>{ "A", "B", "C" };
	auto const& s = std::for_each( v1.begin(), v1.end(), Func{} ).res_;

	std::cout << "String: " << s << '\n';

	//-------------------------------------------------------------------------

	auto v2 = std::vector<std::string>{ "A", "B", "C" };
	auto r = std::views::iota( size_t{ 0 }, v2.size() );

	// Thread safe! mutate a reference object by unique index.
	std::for_each( std::execution::par, r.begin(), r.end(), [ &v2 ] ( size_t i )
				   {
					   v2[i] += std::to_string( i + 1 );
				   } );

	// The overhead incurred by copying data back and forth
	// between the CPU and the GPU is one of the reasons
	// why GPUs are more suitable for batch processing tasks where throughput is more important than latency.
}