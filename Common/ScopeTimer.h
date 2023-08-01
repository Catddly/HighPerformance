#pragma once

#include <chrono>

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
    inline ~ScopedTimer()
    {
        using namespace std::chrono;
        auto stop = ClockType::now();
        auto duration = ( stop - start_ );
        auto us = duration_cast<microseconds>( duration ).count();
        std::cout << us << " us " << function_name_ << '\n';
    }

private:
    const char* function_name_{};
    const ClockType::time_point start_{};
};