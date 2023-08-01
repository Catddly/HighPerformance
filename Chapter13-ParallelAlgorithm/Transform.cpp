#include <thread>
#include <vector>
#include <algorithm>
#include <future>
#include <numeric>

#include "ScopeTimer.h"

//-------------------------------------------------------------------------

template <typename SrcIt, typename DstIt, typename Func>
auto par_transform_naive( SrcIt first, SrcIt last, DstIt dst, Func f )
{
    auto n = static_cast<size_t>( std::distance( first, last ) );
    auto n_cores = size_t{ std::thread::hardware_concurrency() };
    auto n_tasks = std::max( n_cores, size_t{ 1 } );
    auto chunk_sz = ( n + n_tasks - 1 ) / n_tasks;
    auto futures = std::vector<std::future<void>>{};

    // Process each chunk on a separate
    for ( auto i = 0ul; i < n_tasks; ++i )
    {
        auto start = chunk_sz * i;
        if ( start < n )
        {
            auto stop = std::min( chunk_sz * ( i + 1 ), n );
            auto fut = std::async( std::launch::async, [ first, dst, start, stop, f ] ()
            {
                std::transform( first + start, first + stop, dst + start, f );
            } );
            futures.emplace_back( std::move( fut ) );
        }
    }

    // Wait for each task to finish
    for ( auto&& fut : futures )
    {
        fut.wait();
    }
}

auto setup_fixture( int n )
{
    auto src = std::vector<float>( n );
    std::iota( src.begin(), src.end(), 1.0f ); // Values from 1.0 to n

    auto dst = std::vector<float>( src.size() );
    auto transform_function = [] ( float v )
    {
        auto sum = v;
        for ( auto i = 0; i < 500; ++i )
        {
            sum += ( i * i * i * sum );
        }
        return sum;
    };
    return std::tuple{ src, dst, transform_function };
}

void Transform()
{
    // for small data set, sequential version is much faster than parallel version.
    // for big data set, the adavantage of using parallel goes on stage.

    // this is a ideal case where data is evenly slicing into data set.
    // However, for real situation, the execution time of a task might not be equal.
    // (i.e. computation time is not proportional to chunk size)
    // If the application and/or the operating system has other processes to handle, the operation will not process all chunks in parallel.
    {
        auto [src, dst, func] = setup_fixture( 1'000'000 );

        auto timer = ScopedTimer{ "parallel" };
        par_transform_naive( src.begin(), src.end(), dst.begin(), func );
    }

    {
        auto [src, dst, func] = setup_fixture( 1'000'000 );

        auto timer = ScopedTimer{ "sequential" };
        std::transform( src.begin(), src.end(), dst.begin(), func );
    }
}