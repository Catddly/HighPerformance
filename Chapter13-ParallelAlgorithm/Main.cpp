#include "MainEntryHelper.h"

// Note: The efficiency also depends on the problem size and the number of cores.
// For example, a parallel algorithm may perform very poorly
// on small data sets due to the overhead incurred by the added complexity of a parallel algorithm.
// Likewise, executing a program on a great many cores might hit other bottlenecks in the computer such as memory bandwidth.
// We say that a parallel algorithm scales if the efficiency stays constant when we change the number of cores and/or the size of the input.

// speed up = T1 / Tn.
// where T1 is the origin execurtion time of algorithm
//       Tn is the execurtion time of algorithm using n cores on CPU or GPU

// efficiency = T1 / (Tn * n)
// when efficiency = 1.0, it means that parallel optimization take full advantages.

// Use Amdahl's law to compute the maximum speedup of a program:
// Maximum speedup = 1 / ( Fpar / n + Fseq )
//                 = 1 / ( Fpar / n + ( 1 - Fpar ) )
// where Fseq is the fraction of the program that can only be executed sequentially.
//       Fpar is the fraction of the program that can be executed in parallel.

// Manually Header
void Transform();
void DivideAndConquer();
void CountIf();
void CopyIf();
void StdLibrary();

int main()
{
	Entry( Transform );
	Entry( DivideAndConquer );
	Entry( CountIf );
	Entry( CopyIf );
	Entry( StdLibrary );
}