#include <vector>
#include <string>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <list>

// Views in the Ranges library are lazy evaluated iterations over a range.

// Advantages of view:
// 1. Lazy Evaluated.
// 2. Composable.
// 3. Flexibility.
// 4. Code Readability.
// 5. No copy is made! (Efficiency)

// Views are non-owning ranges with complexity guarantees.
// Views don't mutate the underlying container. A view is simply a proxy object that, when iterated, looks like a mutated container.
// Views can be materialized into containers.

struct Student
{
    int year_{};
    int score_{};
    std::string name_{};
    // ...
};

auto max_value( auto&& range )
{
    const auto it = std::ranges::max_element( range );
    return it != range.end() ? *it : 0;
}

auto get_max_score( const std::vector<Student>& students, int year )
{
    const auto by_year = [ = ] ( auto&& s )
    {
        return s.year_ == year;
    };
    return max_value( students
                      | std::views::filter( by_year )
                      | std::views::transform( &Student::score_ ) );
}

auto get_max_score_by_views( const std::vector<Student>& s, int year )
{
    auto by_year = [ = ] ( const auto& s )
    {
        return s.year_ == year;
    };

    auto v1 = std::ranges::ref_view{ s }; // Wrap container in a view
    auto v2 = std::ranges::filter_view{ v1, by_year };
    auto v3 = std::ranges::transform_view{ v2, &Student::score_ };
    auto it = std::ranges::max_element( v3 );
    return it != v3.end() ? *it : 0;
}

// To materialize some views
auto to_vector( auto&& r )
{
    std::vector<std::ranges::range_value_t<decltype( r )>> v;
    if constexpr ( std::ranges::sized_range<decltype( r )> )
    {
        v.reserve( std::ranges::size( r ) );
    }
    std::ranges::copy( r, std::back_inserter( v ) );
    return v;
}

void LazyEvalViews()
{
    auto students = std::vector<Student>{
        { 3, 120, "Niki" },
        { 2, 140, "Karo" },
        { 3, 190, "Sirius" },
        { 2, 110, "Rani" },
    };

    auto max_score = get_max_score( students, 2 );
    std::cout << "Max Score: " << max_score << "\n";

    auto numbers = std::vector{ 1, 2, 3, 4 };
    auto square = [] ( auto v )
    {
        std::cout << "\nInvoke Fuunction square()!\n";
        return v * v;
    };
    auto squared_view = std::views::transform( numbers, square ); // this is only a proxy object
    std::cout << "views::transform()" << "\n";

    for ( auto s : squared_view ) // The square lambda is invoked here
    {
        std::cout << s << " ";
    }
    std::cout << "\n";

    auto list_of_lists = std::vector<std::vector<int>>{
        { 1, 2 },
        { 3, 4, 5 },
        { 5 },
        { 4, 3, 2, 1 }
    };
    auto flattened_view = std::views::join( list_of_lists );
    for ( auto v : flattened_view )
        std::cout << v << " ";
    std::cout << "\n";

    auto max_value = *std::ranges::max_element( flattened_view );
    std::cout << "Max value in joined views: " << max_value << "\n";

    {
        // You can think of a range adaptor as a global stateless object that has two functions implemented:
        // operator()() and operator|().
        // Both functions construct and return view objects.

        auto by_year = [ = ] ( const auto& s )
        {
            return s.year_ == 2;
        };

        auto s = std::vector<Student>{
            { 3, 120, "Niki" },
            { 2, 140, "Karo" },
            { 3, 190, "Sirius" },
            { 2, 110, "Rani" },
        };

        using namespace std::views; // range adaptors live in std::views
        auto scores = s | filter( by_year ) | transform( &Student::score_ ); // fast way to build a composed view!
    }

    std::cout << "\n";

    auto ints = std::list{ 2, 3, 4, 2, 1 };
    auto to_string_view = ints
        | std::views::transform( [] ( auto i )
    {
        return std::to_string( i );
    } );
    auto strings = to_vector( std::move( to_string_view ) );

    for ( auto const& str : strings )
        std::cout << str << " ";
    std::cout << "\n";

    std::cout << "\n";

    auto vec = std::vector{ 4, 2, 7, 1, 2, 6, 1, 5 };
    auto is_odd = [] ( auto i )
    {
        return i % 2 == 1;
    };
    auto odd_numbers = vec | std::views::filter( is_odd );

    //std::ranges::sort( odd_numbers ); // Doesn't compile
    // no choice by materializing first (Similar to Rust functional programming)
    auto v = to_vector( odd_numbers );
    std::ranges::sort( v );

    for ( auto const& e : v )
        std::cout << e << " ";
    std::cout << "\n";

    std::cout << "\n";

    vec = std::vector{ 4, 2, 7, 1, 2, 6, 1, 5 };
    auto first_half = vec | std::views::take( vec.size() / 2 );
    std::ranges::sort( first_half );

    for ( auto const& e : vec )
        std::cout << e << " ";
    std::cout << "\n";

    std::cout << "\n";
}