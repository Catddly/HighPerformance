#include "MainEntryHelper.h"

// Note: Using proxy objects enables optimizations to occur under the hood, thereby leaving the exposed interfaces intact.
// Useful to hide optimization behind for the users of the library.

// Manually Header
void LazyAndEager();
void PostpondComputation();
void ExpressionTemplate();

int main()
{
	Entry( LazyAndEager );
	Entry( PostpondComputation );
	Entry( ExpressionTemplate );
}