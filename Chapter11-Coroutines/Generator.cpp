#include <coroutine>
#include <iostream>
#include <algorithm>
#include <array>
#include <vector>
#include <cassert>
#include <ranges>
#include <fstream>

template <typename T>
class MyGenerator
{
    struct Promise
    { 
        T value_;
        auto get_return_object() -> MyGenerator
        {
            using Handle = std::coroutine_handle<Promise>;
            return MyGenerator{ Handle::from_promise( *this ) };
        }
        auto initial_suspend()
        {
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }
        void return_void()
        {}
        void unhandled_exception()
        {
            throw;
        }
        auto yield_value( T&& value )
        {
            value_ = std::move( value );
            return std::suspend_always{};
        }
        auto yield_value( const T& value )
        {
            value_ = value;
            return std::suspend_always{};
        }
    };

    struct Sentinel
    {};

    struct Iterator
    {
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        std::coroutine_handle<Promise> h_;  // Data member

        Iterator& operator++()
        {
            h_.resume();
            return *this;
        }
        void operator++( int )
        {
            ( void )operator++();
        }
        T operator*() const
        {
            return h_.promise().value_;
        }
        T* operator->() const
        {
            return std::addressof( operator*() );
        }
        bool operator==( Sentinel ) const
        {
            return h_.done();
        }
    };

    std::coroutine_handle<Promise> h_;
    explicit MyGenerator( std::coroutine_handle<Promise> h ) : h_{ h }
    {}
public:
    using promise_type = Promise;
    MyGenerator( MyGenerator&& g ) : h_( std::exchange( g.h_, {} ) )
    {}
    ~MyGenerator()
    {
        if ( h_ )
        {
            h_.destroy();
        }
    }
    auto begin()
    {
        h_.resume();
        return Iterator{ h_ };
    }
    auto end()
    {
        return Sentinel{};
    }
};

template <typename T>
auto seq() -> MyGenerator<T>
{
    for ( T i = {};; ++i )
    {
        co_yield i;
    }
}

template <typename T>
auto take_until( MyGenerator<T>& gen, T value ) -> MyGenerator<T>
{
    for ( auto&& v : gen )
    {
        if ( v == value )
        {
            co_return;
        }
        co_yield v;
    }
}

template <typename T>
auto add( MyGenerator<T>& gen, T adder ) -> MyGenerator<T>
{
    for ( auto&& v : gen )
    {
        co_yield v + adder;
    }
}

template <typename T>
auto lin_value( T start, T stop, size_t index, size_t n )
{
    assert( n > 1 && index < n );
    const auto amount = static_cast<T>( index ) / ( n - 1 );
    const auto v = std::lerp( start, stop, amount ); // C++20
    return v;
}

// eager linear range
template <typename T>
auto lin_space_eager( T start, T stop, size_t n )
{
    auto v = std::vector<T>{};
    for ( auto i = 0u; i < n; ++i )
        v.push_back( lin_value( start, stop, i, n ) );
    return v;
}

// using callback (lazy)
template <typename T, typename F> requires std::invocable<F&, const T&> // C++20 
void lin_space( T start, T stop, std::size_t n, F&& f )
{
    for ( auto i = 0u; i < n; ++i )
    {
        const auto y = lin_value( start, stop, i, n );
        f( y );
    }
}

template <typename T>
struct LinSpace
{
    LinSpace( T start, T stop, std::size_t n )
        : begin_{ start, stop, 0, n }, end_{ n }
    {}
    struct Iterator
    {
        using difference_type = void;
        using value_type = T;
        using reference = T;
        using pointer = T*;
        using iterator_category = std::forward_iterator_tag;
        void operator++()
        {
            ++i_;
        }
        T operator*()
        {
            return lin_value( start_, stop_, i_, n_ );
        }
        bool operator==( std::size_t i ) const
        {
            return i_ == i;
        }
        T start_{};
        T stop_{};
        std::size_t i_{};
        std::size_t n_{};
    };
    auto begin()
    {
        return begin_;
    }
    auto end()
    {
        return end_;
    }
private:
    Iterator begin_{};
    std::size_t end_{};
};

// using iterator (lazy)
template <typename T>
auto lin_space_iterator( T start, T stop, std::size_t n )
{
    return LinSpace{ start, stop, n };
}

// using ranges (lazy)
template <typename T>
auto lin_space_ranges( T start, T stop, std::size_t n )
{
    return 
        std::views::iota( std::size_t{ 0 }, n ) |
        std::views::transform( [ = ] ( auto i )
    {
        return lin_value( start, stop, i, n );
    } );
}

// using coroutine (lazy)
template <typename T>
auto lin_space_coroutine( T start, T stop, std::size_t n ) -> MyGenerator<T>
{
    for ( auto i = 0u; i < n; ++i )
    {
        co_yield lin_value( start, stop, i, n );
    }
}

//-------------------------------------------------------------------------

// turn inverted index into gap encoding( delta encoding )
template <typename Range>
auto gap_encode( Range& ids ) -> MyGenerator<int>
{
    auto last_id = 0;
    for ( auto id : ids )
    {
        const auto gap = id - last_id;
        last_id = id;
        co_yield gap;
    }
}

template <typename Range>
auto gap_decode( Range& gaps ) -> MyGenerator<int>
{
    auto last_id = 0;
    for ( auto gap : gaps )
    {
        const auto id = gap + last_id;
        co_yield id;
        last_id = id;
    }
}

auto variable_encode_num( int n ) -> MyGenerator<std::uint8_t>
{
    for ( auto cont = std::uint8_t{ 0 }; cont == 0;)
    {
        auto b = static_cast<std::uint8_t>( n % 128 );
        n = n / 128;
        cont = ( n == 0 ) ? 128 : 0; // if have _NOT_ more bits, set continuation bit to 1.
        co_yield ( b + cont );
    }
}

template <typename Range>
auto variable_encode( Range& r ) -> MyGenerator<std::uint8_t>
{
    for ( auto n : r )
    {
        auto bytes = variable_encode_num( n );
        for ( auto b : bytes )
        {
            co_yield b;
        }
    }
}

template <typename Range>
auto variable_decode( Range& bytes ) -> MyGenerator<int>
{
    auto n = 0;
    auto weight = 1;
    for ( auto b : bytes )
    {
        if ( b < 128 ) // Check continuation bit
        {
            n += b * weight;
            weight *= 128;
        }
        else
        {
            // Process last byte and yield
            n += ( b - 128 ) * weight;
            co_yield n;
            n = 0;       // Reset
            weight = 1;  // Reset
        }
    }
}

template <typename Range>
auto compress( Range& ids ) -> MyGenerator<int>
{
    auto gaps = gap_encode( ids );
    auto bytes = variable_encode( gaps );
    for ( auto b : bytes )
    {
        co_yield b;
    }
}

template <typename Range>
auto decompress( Range& bytes ) -> MyGenerator<int>
{
    auto gaps = variable_decode( bytes );
    auto ids = gap_decode( gaps );
    for ( auto id : ids )
    {
        co_yield id;
    }
}

template <typename Range>
void write( const std::string& path, Range& bytes )
{
    auto out = std::ofstream{ path, std::ios::out | std::ofstream::binary };
    std::ranges::copy( bytes.begin(), bytes.end(), std::ostreambuf_iterator<char>( out ) );
}

auto read( std::string path ) -> MyGenerator<std::uint8_t>
{
    auto in = std::ifstream{ path, std::ios::in | std::ofstream::binary };
    auto it = std::istreambuf_iterator<char>{ in };
    const auto end = std::istreambuf_iterator<char>{};
    for ( ; it != end; ++it )
    {
        co_yield *it;
    }
}

void Generator()
{
    // generators can be composed
    auto s = seq<int>();
    auto t = take_until<int>( s, 10 );
    auto a = add<int>( t, 3 );

    const auto v = std::array{ 5, 6, 7 };
    std::cout << std::boolalpha << std::ranges::includes( a, v ) << "\n";

    // The generators are lazily evaluated,
    // and compiler have the ability to optimize return value (sum) to evaluate at compile time!!!
    int sum = 0;
    for ( auto&& v : a )
    {
        sum += v;
    }

    std::cout << sum << "\n";

    //-------------------------------------------------------------------------

    for ( auto v : lin_space_eager( 2.0, 3.0, 5 ) )
    {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    //-------------------------------------------------------------------------

    // this lambda can return a bool to indicate if user want more values.
    auto print = [] ( auto v )
    {
        std::cout << v << ", ";
    };
    lin_space( -1.f, 1.f, 5, print );
    std::cout << '\n';

    //-------------------------------------------------------------------------

    for ( auto v : lin_space_iterator( 2.0, 3.0, 5 ) )
    {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    //-------------------------------------------------------------------------

    for ( auto v : lin_space_ranges( 2.0, 3.0, 5 ) )
    {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    //-------------------------------------------------------------------------

    for ( auto v : lin_space_coroutine( 2.0, 3.0, 5 ) )
    {
        std::cout << v << ", ";
    }
    std::cout << '\n';

    //-------------------------------------------------------------------------

    auto ids = std::array{ 10, 11, 12, 14 };
    auto gaps = gap_encode( ids );
    auto decoded = gap_decode( gaps );

    for ( auto&& d : decoded )
    {
        std::cout << d << ", ";
    }
    std::cout << '\n';

    // Variable byte encoding is a very common compression technique.
    // UTF-8 and MIDI message are some of the well-known encodings that uses this technique.
    // We use 7-bits of each byte for the actual payload.
    // The first bit of each byte represents a continuation bit.
    // It is set to 0 if there are more bytes to read, or 1 for the last byte of the encoded number.

    {
        auto documents = std::vector{ 367, 438, 439, 440 };
        auto bytes = compress( documents );
        write( "values.bin", bytes );
    }

    {
        auto bytes = read( "values.bin" );
        auto documents = decompress( bytes );
        for ( auto doc : documents )
        {
            std::cout << doc << ", ";
        }
    }
}