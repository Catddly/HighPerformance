#include <vector>
#include <string>
#include <iostream>

// Note: It is usually harm to allocate small object using heap, try to use stack to store this objects.

//auto allocated = size_t{ 0 };
//// Overload operator new and delete to track allocations 
//void* operator new( size_t size )
//{
//	void* p = std::malloc( size );
//	allocated += size;
//	return p;
//}
//
//void operator delete( void* p ) noexcept
//{
//	return std::free( p );
//}

void SmallObjectOptimization()
{
	auto smallVec = std::vector{ 2, 3 }; // bad
	int smallArray[] = { 2, 3, 0, 0 }; // good

	auto shortStr = std::string{ "" };

	std::cout << sizeof( shortStr ) << "\n";

	// try this in release mode
	//std::cout << "stack space = " << sizeof( shortStr )
	//	<< ", heap space = " << allocated
	//	<< ", capacity = " << shortStr.capacity() << '\n';
}