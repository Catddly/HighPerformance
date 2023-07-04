#include <iostream>

// Five Implementation:
// 1. Destructor
// 2. Copy Ctor
// 3. Copy Operator
// 4. Move Ctor
// 5. Move Operator

// When you try to implement your own Destructor in a class,
// think of Rule Of Five.

// So, when is the compiler allowed to move objects instead of copying?
// As a short answer, the compiler moves an object when the object can be categorized as an rvalue.
// The term rvalue might sound complicated,
// but in essence it is just an object that is not tied to a named variable,
// for either of the following reasons:
//
// 1. It's coming straight out of a function.
// 2. We make a variable an rvalue by using std::move().

// Writing your classes so that they don't require any of these special member functions to be explicitly written (or default - declared) is often referred to as the rule of zero.

class NonCopyable
{
public:

	NonCopyable() = default;
	NonCopyable( int a )
		: m_a( a )
	{}
	~NonCopyable() = default;

	NonCopyable( NonCopyable const& ) = delete;
	NonCopyable& operator=( NonCopyable const& ) = delete;

	NonCopyable( NonCopyable&& ) noexcept : m_a( 0 ) { std::cout << "Move Ctor!\n"; }
	NonCopyable& operator=( NonCopyable&& ) noexcept { std::cout << "Move Operator!\n"; }

private:

	int				m_a;
};

//class Button
//{
//public:
//	Button()
//	{} // Same as before
//
//	// Nothing here, the compiler generates everything automatically! 
//	// ...
//};

//class Button
//{
//public:
//	Button()
//	{}
//  // Custom destructor, move ctor/operator will not be generated, always make copy.
//	~Button()
//		std::cout << "destructed\n"
//}
//// ...
//};

// NRVO (Named Return Value Optimization)
NonCopyable ReturnNonCopyableNRVO()
{
	auto res = NonCopyable{ 2 };
	return res;
}

// RVO (Return Value Optimization)
NonCopyable ReturnNonCopyableRVO()
{
	return NonCopyable{ 2 };
}

struct Point
{
	int x_, y_;

	// Empty destructor, don't use!
	// Writing an empty destructor can prevent the compiler from implementing certain optimizations.
	~Point() {}
};

// gcc 13.1 compile result:

// Asm in empty destructor version:
// Compiler use for loop to do copy
// 
// xor eax, eax
// .L2:
// mov rdx, QWORD PTR[rdi + rax]
// mov QWORD PTR[rsi + rax], rdx
// add rax, 8
// cmp rax, 512
// jne.L2
// rep ret

// Asm in default destructor / no custom destructor version:
// Compiler optimizes using memmove() instead of loops
// 
// mov rax, rdi
// mov edx, 512
// mov rdi, rsi
// mov rsi, rax
// jmp memmove

auto copy( Point* src, Point* dst )
{
	std::copy( src, src + 64, dst );
}

void RuleOfFive()
{
	auto nonCopyable0 = ReturnNonCopyableNRVO();
	auto nonCopyable1 = ReturnNonCopyableRVO();

	//auto makeCopy = nonCopyable0; // [ERROR] Can not be copied
}