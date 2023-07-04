#include <vector>
#include <algorithm>
#include <iostream>

void PartialSort()
{
	auto v = std::vector<int>( 12 );
	std::ranges::generate( v, std::rand );
	auto iter = v.begin() + v.size() / 2;

	// O(nlogn)
	//std::ranges::sort( v );
	
	// only sort the nth element O(n)
	std::nth_element( v.begin(), iter - 1, v.end() );

	// sort until iter (exclude iter) O(nlogm)
	std::partial_sort( iter - 1, iter + 2, v.end() );

	// by combining sort(), nth_element(), partial_sort() we can efficiently sort partial elements in the range.

	std::ranges::for_each( v, [] ( auto const& val )
	{
		std::cout << val << " ";
	});
	std::cout << "\n";
}