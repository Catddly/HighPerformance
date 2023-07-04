#include "MainEntryHelper.h"

//-------------------------------------------------------------------------
// Performance Concern When Using Std Containers:
// 
// 1. Balancing between complexity guarantees and overhead.
//    It's equally important to remember that each container is afflicted with an overhead cost,
//    which has a bigger impact on the performance for smaller datasets.
//    The complexity guarantees only become interesting for sufficiently large datasets.
// 2. Knowing and using the appropriate API functions.
//    Api is well-desigend should have their advantages and disadvantages.
//    Don't use detours like the ones shown earlier just because you haven't learned the full API or because you have old habits of doing things in a certain way.
//-------------------------------------------------------------------------

// Manually Header
void ComputerMemory();
void Container();
void View();
void ParallelArray();

int main()
{
	Entry( ComputerMemory );
	Entry( Container );
	Entry( View );
	Entry( ParallelArray );
}