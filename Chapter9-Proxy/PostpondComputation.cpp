#include <cmath>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <vector>

class LengthProxy
{
public:
    LengthProxy( float x, float y ) : squared_{ x * x + y * y }
    {}
    bool operator==( const LengthProxy& other ) const = default;
    auto operator<=>( const LengthProxy& other ) const = default;
    friend auto operator<=>( const LengthProxy& proxy, float len )
    {
        return proxy.squared_ <=> len * len; // C++20
    }

    // Allow implicit cast to float
    // here && is _important_, we use this to restrict that user can only fetch the actul length from LengthProxy by rvalue.
    // (i.e. adding && to a member function means that it can only be called at one object at a time)
    operator float() const &&
    {
        return std::sqrt( squared_ );
    }
private:
    float squared_{};
};

class Vec2D
{
public:
    Vec2D( float x, float y ) : x_{ x }, y_{ y }
    {}

    auto length_squared() const
    {
        return x_ * x_ + y_ * y_;
    }

    auto length() const
    {
        //auto squared = x_ * x_ + y_ * y_;
        //return std::sqrt( squared ); // std::sqrt() is a slow operation
        return LengthProxy{ x_, y_ };
    }
private:
    float x_{};
    float y_{};
};

auto min_length_slow( const std::ranges::range auto& r ) -> float
{
    assert( !r.empty() );
    auto cmp = [] ( auto&& a, auto&& b )
    {
        return a.length() < b.length(); // by using LengthProxy, now the slow version is the fast version!
    };
    auto it = std::ranges::min_element( r, cmp );
    return it->length();
}

auto min_length_fast( const std::ranges::range auto& r ) -> float
{
    assert( !r.empty() );
    auto cmp = [] ( auto&& a, auto&& b )
    {
        return a.length_squared() < b.length_squared(); // Faster
    };
    auto it = std::ranges::min_element( r, cmp );
    // we want the length after all comparison
    return it->length(); // But remember to use length() here!
}

template <typename T>
struct ContainsProxy
{
    const T& value_;
};

template <typename T>
auto operator|( const std::ranges::range auto& r, const ContainsProxy<T>& proxy )
{
    const auto& v = proxy.value_;
    return std::ranges::find( r, v ) != r.end();
}

namespace proxy
{
    template <typename T>
    inline auto contains( const T& v )
    {
        return ContainsProxy<T>{v};
    }
}

void PostpondComputation()
{
    // Notice that when comparing length, we actually doesn't need to call std::sqrt() to compute the square root!
    // we can just compare a^2 + b^2 with x^2 + y^2

    auto a = Vec2D{ 3, 4 };
    auto b = Vec2D{ 4, 4 };
    auto shortest = a.length() < b.length() ? a : b; // this comparison take advantages from LengthProxy!
    // we can _NOT_ use auto anymore due to the proxy object
    float length = shortest.length();

    std::cout << length << "\n";

    // this case using LengthProxy will cause bad performance!
    auto user_length = a.length(); // user didn't realize user_length is a LengthProxy

    // By adding && to operator float() to LengthProxy, this code won't be compiled and performance is being saved!
    //float len0 = user_length;
    //float len1 = user_length; // std::sqrt() is call again! But in this case user is just want to copy the value!
    //float len2 = user_length; // std::sqrt() is call again! But in this case user is just want to copy the value!

    //-------------------------------------------------------------------------

    // More proxy optimization
    // expression template

    //-------------------------------------------------------------------------

    auto numbers = std::vector{ 1, 3, 5, 7, 9 };
    auto seven = 7;
    bool has_seven = numbers | proxy::contains( seven );

    if ( has_seven )
    {
        std::cout << "Vector has seven!\n";
    }   
}