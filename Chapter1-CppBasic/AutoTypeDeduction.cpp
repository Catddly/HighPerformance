#include "TypeName.h"

#include <iostream>
#include <type_traits>

class Value
{
public:
	// Deduced type
	auto& ValDeduced() const { return m_a; }

	// Trailing type
	auto ValTrailing() -> int const& { return m_a; }

	// Explicit type
	int const& ValExplicit() const { return m_a; }

private:

	int m_a;
};

// decltype(auto) is most commonly used for forwarding exact type, since decltype(auto) will not decay type.
// Example Below:

auto TakeValue( int refInt )
{
	return refInt;
}

auto TakeRefWrong( int& refInt )
{
	return refInt;
}

decltype(auto) TakeRefCorrect( int& refInt )
{
	return refInt;
}

void AutoTypeDeduction()
{
	int a = 1;

	std::cout << "TakeValue()      return type: " << TYPE_CATEGORY( TakeValue( a ) ) << "\n";
	std::cout << "TakeRefWrong()   return type: " << TYPE_CATEGORY( TakeRefWrong( a ) ) << "\n";
	std::cout << "TakeRefCorrect() return type: " << TYPE_CATEGORY( TakeRefCorrect( a ) ) << "\n";
}