#include <algorithm>
#include <future>
#include <thread>
#include <numeric>
#include <iostream>

// Divide and Conquer: if the input range is smaller than a specified threshold,
// the range is processed; otherwise, the range is split into two parts:
//
// - The first part is processed on a newly branched task
// - The other part is recursively processed at the calling thread

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

//-------------------------------------------------------------------------

// recursive version (divide and conquer)
template <typename SrcIt, typename DstIt, typename Func>
auto par_transform_dac( SrcIt first, SrcIt last, DstIt dst,
                    Func func, size_t chunk_sz )
{
    const auto n = static_cast<size_t>( std::distance( first, last ) );
    if ( n <= chunk_sz )
    {
        std::transform( first, last, dst, func );
        return;
    }

    const auto src_middle = std::next( first, n / 2 );

    // Branch of first part to another task
    auto future = std::async( std::launch::async, [ =, &func ]
    {
        par_transform_dac( first, src_middle, dst, func, chunk_sz );
    } );

    // Recursively handle the second part
    const auto dst_middle = std::next( dst, n / 2 );
    par_transform_dac( src_middle, last, dst_middle, func, chunk_sz );

    future.wait();
}

auto setup( int n )
{
    auto src = std::vector<float>( n );
    std::iota( src.begin(), src.end(), 1.0f );  // From 1.0 to n
    auto dst = std::vector<float>( src.size() );

    // now the execution time of each task is completely uneven.
    auto transform_function = [] ( float v )
    {
        auto sum = v;
        auto n = v / 20'000; // The larger v is, the more to compute
        for ( auto i = 0; i < n; ++i ) 
        {        
            sum += ( i * i * i * sum );
        }
        return sum;
    };
    return std::tuple{ src, dst, transform_function };
}

void DivideAndConquer()
{
    // With larger chunks, the performance is bottlenecked in the time it takes to process the final chunks,
    // whereas too small chunks result in too much overhead in creating and invoking tasks compared to the computation.
    
    // More threads doesn't mean better, use thread pool is limit the number of worker threads.
    {
        auto [src, dst, func] = setup( 1'000'000 );

        auto timer = ScopedTimer{ "parallel" };
        par_transform_dac( src.begin(), src.end(), dst.begin(), func, 100'000 );
    }
}