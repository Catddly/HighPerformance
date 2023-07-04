#include <algorithm>
#include <vector>
#include <iostream>

void DifferenceOfCpp20( )
{
	auto values = std::vector{ 9, 2, 5, 3, 4 };

	// Sort using the std algorithms. ( Old )
	std::sort( values.begin(), values.end() );

	// Sort using the constrained algorithms under std::ranges ( New )
	std::ranges::sort( values );
	std::ranges::sort( values.begin(), values.end() );

	// Both new sort and old sort is in header <algorithm>

	// Time to use algotithm!
	//for ( auto const& e : values )
	//{
	//	std::cout << e << "\n";
	//}

	std::for_each( values.begin(), values.end(), [] ( auto const& val )
	{
		std::cout << val << "\n";
	} );
}