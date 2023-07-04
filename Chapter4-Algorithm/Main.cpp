#include "MainEntryHelper.h"

// Manually Header
void DifferenceOfCpp20();
void Algorithm();
void IteratorAndRanges();
void PartialSort();

// Note: by using std algorithm, we get the performance improment for free!
// 1. loop unrolling (one inspect into four inspects)
// 2. 'Compare with zero' optimization.

int main()
{
	Entry( DifferenceOfCpp20 );
	Entry( Algorithm );
	Entry( IteratorAndRanges );
	Entry( PartialSort );
}