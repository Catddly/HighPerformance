#include <memory>
#include <iostream>

void MemoryOwnership()
{
	// The slight overhead is incurred by the fact
	// that std::unique_ptr has a non-trivial destructor,
	// which means that (unlike a raw pointer)
	// it cannot be passed in a CPU register when being passed to a function.
	// This makes them slower than raw pointers.
	auto ptr = std::make_unique<int>( 444 );
	std::cout << *ptr << "\n";

	auto weak_i = std::weak_ptr<double>{};
	{
		auto val = std::make_shared<double>( 42.0 );
		std::cout << *val << "\n";

		weak_i = std::weak_ptr<double>{ val };
	}

	if ( auto shared_i = weak_i.lock() )
	{
		// We managed to convert our weak pointer to a shared pointer 
		std::cout << *shared_i << '\n';
	}
	else
	{
		std::cout << "weak_i has expired, shared_ptr was nullptr\n";
	}
}