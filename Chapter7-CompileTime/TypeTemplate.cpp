#include <type_traits>

// pow_n accepts any number type 
// using abbreviated function templates (C++20)
auto pow_n( const auto& v, int n )
{
    typename std::remove_cvref<decltype( v )>::type product = { 1 }; // C++20
    // auto product = decltype( v ){ 1 };
    for ( int i = 0; i < n; ++i )
    {
        product *= v;
    }
    return product;
}

// Rectangle can be of any type 
template <typename T>
class Rectangle
{
public:
    Rectangle( T x, T y, T w, T h ) : x_{ x }, y_{ y }, w_{ w }, h_{ h }
    {}
    auto area() const
    {
        return w_ * h_;
    }
    auto width() const
    {
        return w_;
    }
    auto height() const
    {
        return h_;
    }
private:
    T x_{}, y_{}, w_{}, h_{};
};

template <typename T>
auto is_square( const Rectangle<T>& r )
{
    return r.width() == r.height();
}

// template argument can be int
template <int N, typename T>
auto const_pow_n( const T& v )
{
    static_assert( N > 0, "N must be positive" ); // detect problems at compile time
    auto product = T{ 1 };
    for ( int i = 0; i < N; ++i )
    {
        product *= v;
    }
    return product;
}

// template specialization
template<>
auto const_pow_n<2, int>( const int& v )
{
    return v * v;
}

// The reason we cannot apply partial template specialization to functions is that functions can be overloaded (and classes cannot).
// If we were allowed to mix overloads and partial specialization, it would be very hard to comprehend.
// template partial specialization (can only be applied to class)

// [ERROR] This won't be compiled!
//template <typename T>
//auto const_pow_n<T, 2>( const T& v )
//{
//    return v * v;
//}

void TypeTemplate()
{
    auto _x = pow_n<float>( 2.0f, 3 ); // x is a float 
    auto _y = pow_n<int>( 3, 3 );      // y is an int 

    // template argument deduction (here in this case it is a deducible context)
    auto x = pow_n( 2.0f, 3 ); // x is a float 
    auto y = pow_n( 3, 3 );    // y is an int

    auto r1 = Rectangle<float>{ 2.0f, 2.0f, 4.0f, 4.0f };
    //class template argument deduction( CTAD )
    auto r2 = Rectangle{ -2, -2, 4, 4 };

    auto x2 = const_pow_n<2>( 4.0f );   // Square
    auto x3 = const_pow_n<3>( 4.0f );   // Cube

    // Before C++20, it was common to see decltype in the body of generic lambdas.
    // However, it is now possible to avoid the rather inconvenient decltype by adding explicit template parameters to generic lambdas.
    auto generic_lambda = [] <typename T> ( T const& val, int n )
    {
        auto product = T{ 1 };
        for ( int i = 0; i < n; ++i )
        {
            product *= val;
        }
        return product;
    };
}