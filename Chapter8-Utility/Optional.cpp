#include <cassert>
#include <optional>
#include <algorithm>
#include <vector>
#include <iostream>

// If you are in the habit of representing optional values using pointers,
// designing APIs using out parameters, or adding special null states in enums,
// it's time to add std::optional to your toolbox,
// as it provides an efficient and safe alternative to these anti-patterns.

struct Hat
{
    /* ... */
};

class Head
{
public:
    Head()
    {
        // hat_ is empty by default
        assert( !hat_ );
    }      
    auto set_hat( const Hat& h )
    {
        hat_ = h;
    }
    auto has_hat() const
    {
        return hat_.has_value();
    }
    auto& get_hat() const
    {
        assert( hat_.has_value() );
        return *hat_;
    }
    auto remove_hat()
    {
        hat_ = {};        // Hat is cleared by assigning to {}
    }
private:
    std::optional<Hat> hat_;
};

void Optional()
{
    auto a = std::optional<int>{};
    auto b = std::optional<int>{};
    auto c = std::optional<int>{ 4 };

    assert( a == b ); // two empty optional is considered equal
    assert( b != c );

    auto d = std::optional<int>{};
    auto d1 = std::optional<float>{};
    auto e = std::optional<int>{ 4 };
    auto e1 = std::optional<float>{ 2.3f };
    auto f = std::optional<int>{ 5 };

    assert( d == d1 );
    assert( e1 < e );
    assert( d < e ); // empty optional is considered less than not empty optional
    assert( e < f );

    auto vec = std::vector<std::optional<int>>{ {3}, {}, {1}, {}, {2} };
    std::sort( vec.begin(), vec.end() );

    for ( auto const& e : vec )
    {
        if ( e.has_value() )
        {
            std::cout << *e << " ";
        }
        else
        {
            std::cout << "{}" << " ";
        }
    }
    std::cout << "\n";
}