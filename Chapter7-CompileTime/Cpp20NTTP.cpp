// Note: NTTP stands for non type template parameter.

// C++20 support two more Non-types as template parameter:
// 1. floatint-points
// 2. literal

struct ClassType
{
    constexpr ClassType( int )
    {}
};

template <ClassType cl>
auto getClassType()
{
    return cl;
}

template <double d>
auto getDouble()
{
    return d;
}

void Cpp20NTTP()
{
    auto c1 = getClassType<ClassType( 2020 )>();

    auto d1 = getDouble<5.5>();
    auto d2 = getDouble<6.5>();
}