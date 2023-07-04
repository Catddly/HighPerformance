#include "MainEntryHelper.h"

// Note: C++ containers are homogenous, meaning that they can only store elements of one single type.
// In a heterogeneous collection, the elements may have different types.

// Manually Header
void Optional();
void Heterogenous();

int main()
{
	Entry( Optional );
	Entry( Heterogenous );
}