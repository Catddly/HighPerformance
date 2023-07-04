// C++ treats memory as a sequence of cells. The size of each cell is 1 byte, and each cell has an address.
// Since accessing a byte by its address is an O(1) operation, from a programmer's perspective,
// it's tempting to believe that each memory cell is equally quick to access.

// Constantly wiping out the cache lines in inner loops might result in very bad performance.
// This is sometimes called cache thrashing.

#include <vector>
#include <iostream>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

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

auto get_l1d_cache_size()
{
    size_t size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = 0;

    GetLogicalProcessorInformation( 0, &buffer_size );
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc( buffer_size );
    GetLogicalProcessorInformation( &buffer[0], &buffer_size );

    for ( i = 0; i != ( buffer_size / sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION ) ); ++i )
    {
        if ( buffer && buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1 )
        {
            size = buffer[i].Cache.Size;
            break;
        }
    }

    free( buffer );
    return size;
}

constexpr auto kL1CacheCapacity = 49152; // The L1 Data cache size (may NOT be the L1 Data cache line size in your computer)
constexpr auto kSize = kL1CacheCapacity / sizeof( int );

using MatrixType = std::vector<std::vector<int>>;

MatrixType data_initialize()
{
    auto matrix = MatrixType{};
    matrix.resize( kSize );
    for ( auto& e : matrix )
    {
        e.resize( kSize );
    }
    return matrix;
}

auto no_cache_thrashing( MatrixType& matrix )
{
    auto counter = 0;
    for ( auto i = 0; i < kSize; ++i )
    {
        for ( auto j = 0; j < kSize; ++j )
        {
            matrix[i][j] = counter++;
        }
    }
}

auto cache_thrashing( MatrixType& matrix )
{
    auto counter = 0;
    for ( auto i = 0; i < kSize; ++i )
    {
        for ( auto j = 0; j < kSize; ++j )
        {
            matrix[j][i] = counter++;
        }
    }
}

void ComputerMemory()
{
    std::cout << "L1d cache size: " << get_l1d_cache_size() << "\n\n";

    auto mat0 = data_initialize();
    {
        ScopedTimer timer( "Normal Accessing" );
        no_cache_thrashing( mat0 );
    }

    {
        ScopedTimer timer( "Cache Thrashing" );
        cache_thrashing( mat0 );
    }
}