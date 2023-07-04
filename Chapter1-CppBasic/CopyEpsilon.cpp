#include <mutex>

int Create()
{
	return 2;
}

class Foo
{};

void CopyEpsilon()
{
	// By using auto, we must initialized the variable before using it.
	// auto i; // [Error] a symbol whose type contains 'auto' must have an initializer.
	auto i = 0;
	auto x = Foo{};
	auto y = Create();

	// Guaranteed Copy Elision introduced in C++17
	// It seems that we create a temporary object of std::mutex and make a copy to z.
	// Guaranteed Copy Elision guarantees that there is no temporary object that needs to be moved or copied in this case.
	auto z = std::mutex{}; // OK since C++17
	std::mutex w{}; // Same as above
}