#include <iostream>

struct Foo
{
	auto func() &&
	{
		std::cout << "Call rvalue only function!\n";
	}
};

void RValueMemberFunction()
{
	auto a = Foo{};
	//a.func(); // [ERROR] Won't compile. a is a lvalue.
	std::move( a ).func();
	Foo{}.func();
}