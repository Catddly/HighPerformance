#include <string>
void CompilerOptimization()
{
	auto x = 0;
	auto y = std::string{};

	// User
	x = 1;
	y = "hello";
	x = 2;

	// unobservable write elimination
	// Compiler (the observable result is the same)
	y = "hello";
	x = 2;

	//-------------------------------------------------------------------------

	auto z = 12;

	// User
	for ( int i = 0; i < 20; ++i )
	{
		// suppose that every time we try to write to z, it read from main memory.
		// this is slow
		z += i;
	}

	// Compiler (the observable result is the same)
	// here r1 is a register from a CPU core
	auto r1 = z;
	
	for ( int i = 0; i < 20; ++i )
	{
		// read and write to register, very fast
		r1 += i;
	}

	// write back to main memory
	z = r1;

	//-------------------------------------------------------------------------

	{
		// User
		auto x = std::string{ "no" };
		auto y = std::string{ "way" };
		auto z = std::string{ "everything" };
	}

	{
		// Compiler
		// maybe we just use z and it is in the register!
		auto z = std::string{ "everything" };
		auto y = std::string{ "way" };
		auto x = std::string{ "no" };
	}
}