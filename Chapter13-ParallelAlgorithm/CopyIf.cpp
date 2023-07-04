#include <atomic>
#include <algorithm>
#include <thread>
#include <vector>
#include <future>

// first approach: use atomic
//-------------------------------------------------------------------------

// this is slow when the pred calculation takes short time.
// The disastrous performance is actually not due to the atomic write index;
// rather, it is because the cache mechanism of the hardware is trashed due to
// several threads writing to the same cache line (as you learned about in Chapter 6, Memory Management).

template <typename SrcIt, typename DstIt, typename Pred>
void inner_par_copy_if_sync( SrcIt first, SrcIt last, DstIt dst,
                             std::atomic_size_t& dst_idx,
                             Pred pred, size_t chunk_sz )
{
    const auto n = static_cast<size_t>( std::distance( first, last ) );
    if ( n <= chunk_sz )
    {
        std::for_each( first, last, [ & ] ( const auto& v )
        {
            if ( pred( v ) )
            {
                auto write_idx = dst_idx.fetch_add( 1 );
                *std::next( dst, write_idx ) = v;
            }
        } );
        return;
    }

    auto middle = std::next( first, n / 2 );
    auto future = std::async( [ first, middle, dst, chunk_sz, &pred, &dst_idx ]
    {
        inner_par_copy_if_sync( first, middle, dst, dst_idx, pred, chunk_sz );
    } );

    inner_par_copy_if_sync( middle, last, dst, dst_idx, pred, chunk_sz );
    future.wait();
}

template <typename SrcIt, typename DstIt, typename Pred>
auto par_copy_if_sync( SrcIt first, SrcIt last, DstIt dst,
                       Pred p, size_t chunk_sz )
{
    auto dst_write_idx = std::atomic_size_t{ 0 };
    inner_par_copy_if_sync( first, last, dst, dst_write_idx, p, chunk_sz );
    return std::next( dst, dst_write_idx );
}

// second approach: split and merge
//-------------------------------------------------------------------------

template <typename SrcIt, typename DstIt, typename Pred>
auto par_copy_if_split( SrcIt first, SrcIt last, DstIt dst,
                        Pred pred, size_t chunk_sz ) -> DstIt
{
    // Part #1: Copy elements in parallel into the destination range
    auto n = static_cast<size_t>( std::distance( first, last ) );
    auto futures = std::vector<std::future<std::pair<DstIt, DstIt>>>{};
    futures.reserve( n / chunk_sz );

    for ( auto i = size_t{ 0 }; i < n; i += chunk_sz )
    {
        const auto stop_idx = std::min( i + chunk_sz, n );
        auto future = std::async( [ =, &pred ]
        {
            auto dst_first = dst + i;
            auto dst_last = std::copy_if( first + i, first + stop_idx, dst_first, pred );
            return std::make_pair( dst_first, dst_last );
        } );

        futures.emplace_back( std::move( future ) );
    }

    // Part #2: Perform merge of resulting sparse range sequentially 
    auto new_end = futures.front().get().second;
    for ( auto it = std::next( futures.begin() ); it != futures.end(); ++it )
    {
        auto chunk_rng = it->get();
        new_end = std::move( chunk_rng.first, chunk_rng.second, new_end );
    }

    return new_end;
}

//-------------------------------------------------------------------------

void CopyIf()
{

}