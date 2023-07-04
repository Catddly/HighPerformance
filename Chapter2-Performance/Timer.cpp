#include <chrono>
#include <iostream>
#include <source_location>

// This is an instruction profiler
class ScopedTimer
{
public:
    using ClockType = std::chrono::steady_clock;
    ScopedTimer( const char* func )
        : function_name_{ func }, start_{ ClockType::now() }
    {}
    ScopedTimer( const ScopedTimer& ) = delete;
    ScopedTimer( ScopedTimer&& ) = delete;
    auto operator=( const ScopedTimer& )->ScopedTimer & = delete;
    auto operator=( ScopedTimer&& )->ScopedTimer & = delete;
    ~ScopedTimer()
    {
        using namespace std::chrono;
        auto stop = ClockType::now();
        auto duration = ( stop - start_ );
        auto ms = duration_cast<milliseconds>( duration ).count();
        std::cout << ms << " ms " << function_name_ << '\n';
    }

private:
    const char* function_name_{};
    const ClockType::time_point start_{};
};

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