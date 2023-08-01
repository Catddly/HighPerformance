#include <algorithm>
#include <future>
#include <thread>
#include <numeric>
#include <iostream>

#include "ScopeTimer.h"

//-------------------------------------------------------------------------

template <typename It, typename Pred>
auto par_count_if( It first, It last, Pred pred, size_t chunk_sz )
{
    auto n = static_cast<size_t>( std::distance( first, last ) );
    if ( n <= chunk_sz )
        return std::count_if( first, last, pred );

    auto middle = std::next( first, n / 2 );

    auto fut = std::async( std::launch::async, [ =, &pred ]
    {
        return par_count_if( first, middle, pred, chunk_sz );
    } );

    auto num = par_count_if( middle, last, pred, chunk_sz );
    return num + fut.get();
}

template <typename It, typename Pred>
auto par_count_if( It first, It last, Pred pred )
{
    auto n = static_cast<size_t>( std::distance( first, last ) );
    auto n_cores = size_t{ std::thread::hardware_concurrency() };

    auto chunk_sz = std::max( n / n_cores * 32, size_t{ 10'000 } );

    return par_count_if( first, last, pred, chunk_sz );
}

auto setup_count_if( int n )
{
    auto src = std::vector<float>( n );
    std::iota( src.begin(), src.end(), 1.0f );  // From 1.0 to n

    auto predicator = [n] ( float val )
    {
        return val > ( n / 2.0 );
    };

    return std::tuple{ src, predicator };
}

void CountIf()
{
    auto [src, pred] = setup_count_if( 10'000'000 );

    auto sum = 0ll;

    {
        auto timer = ScopedTimer{ "parallel count_if" };
        sum = par_count_if(src.begin(), src.end(), pred );
    }
    std::cout << "Sum = " << sum << '\n';
}