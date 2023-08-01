#include <chrono>
#include <iostream>
#include <source_location>

#include "ScopeTimer.h"

#define USE_TIMER 1

#if USE_TIMER
#define MEASURE_FUNCTION() ScopedTimer __timer{ std::source_location::current().function_name() } 
#else 
#define MEASURE_FUNCTION() 
#endif

void MyFunc()
{
    MEASURE_FUNCTION();

    auto total = 0ull;
    for ( int i = 0; i < 100'000'000; ++i )
    {
        total += 12;
    }
}

void Timer()
{
    MyFunc();
}