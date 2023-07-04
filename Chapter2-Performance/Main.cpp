#include "MainEntryHelper.h"
#include <source_location>

//-------------------------------------------------------------------------
// Performance 
// 
// Do NOT tune your code details before you find the right algorithm and data
// structure.
// 
// Common Optimization Steps:
// 1. Define a goal.
// 2. Measure.
// 3. Find the bottlenecks.
// 4. Make an educated guess.
// 5. Optimize.
// 6. Evaluate. Measure again.
// 7. Refactor.
// 
// Performance Properties:
// 1. Latency/response time.
// 2. Throughput.
// 3. I/O bound or CPU bound.
// 4. Power consumption.
// 5. Data aggregation.
// 
// Speedup of execution time:
// Speedup of execution time = Told / Tnew.
// 
// Many CPUs are equipped with hardware performance counters that can provide us
// with metrics such as the number of instructions,
// CPU cycles, branch mispredictions, and cache misses.
// It's good to know that they exist and that there are ready-made tools
// and libraries (accessible through APIs) for all the major operating systems
// to collect Performance Monitoring Counters (PMC) while running a program.
// 
// The support for performance counters varies depending on the CPU and operating system.
// Intel provides a powerful tool called VTune, which can be used for monitoring performance counters.
// FreeBSD offers pmcstat. macOS comes with DTrace and Xcode Instruments.
// Microsoft Visual Studio provides support for collecting CPU counters on Windows.
// 
// 
// There are two main categories of profilers: sampling profilers and instrumentation profilers.
// 
// But sometimes we find a small function or a small block of code that we need to optimize,
// and in those cases, we can use a method called microbenchmarking.
// With this process we create a microbenchmark—a program that runs a small piece of code in isolation from the rest of the program.
// The process of microbenchmarking consists of the following steps:
// 
// 1. Find a hot spot that needs tuning, preferably using a profiler.
// 2. Separate it from the rest of the codeand create an isolated microbenchmark.
// 3. Optimize the microbenchmark.Use a benchmarking framework to testand evaluate the code during optimization.
// 4. Integrate the newly optimized code into the programand measure again to see if the optimizations are relevant when the code runs in a bigger context with more relevant input.
// 
// Say that we are optimizing an isolated part of a program in a microbenchmark.
// The upper limit of the overall speedup of the entire program can then be computed using Amdahl's law.
// 
// Overall Speedup = 1 / ( 1 - p ) + p / s,
// where p is the proportion between execution time of microbenchmark code snippet and the execution time of overall program.
//	     s is the speedup of the code snippet.
//-------------------------------------------------------------------------

// Manually Header
void BigONotation();
void Timer();

int main()
{
	Entry( BigONotation );
	Entry( Timer );
}