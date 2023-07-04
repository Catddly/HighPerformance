#include <vector>
#include <iostream>
#include <numeric> // std::midpoint

// Informal definition for big O notation:
// This is so called Asymptotic complexity.
//
// If f( n ) is a function that specifies the running time of an algorithm with input size n,
// we say that f( n ) is O( g( n ) ) if there is a constant k such that f( n ) <= k * g( n ).

// Amortized running time is used for analyzing a sequence of operations rather than a single one.
// We are still analyzing the worst case, but for a sequence of operations.
// The amortized running time can be computed by first analyzing 
// the running time of the entire sequence and then dividing that by the length of the sequence.

// O( N )
bool linear_search( const std::vector<int>& vals, int key ) noexcept
{
    for ( const auto& v : vals )
    {
        if ( v == key )
        {
            return true;
        }
    }
    return false;
}

// O( LogN )
bool binary_search( const std::vector<int>& a, int key )
{
    auto low = 0;
    auto high = static_cast<int>( a.size() ) - 1;
    while ( low <= high )
    {
        const auto mid = std::midpoint( low, high ); // C++20
        if ( a[mid] < key )
        {
            low = mid + 1;
        }
        else if ( a[mid] > key )
        {
            high = mid - 1;
        }
        else
        {
            return true;
        }
    }
    return false;
}

void BigONotation()
{
    auto vec = std::vector<int>{ 1, 2, 3, 4, 6, 7, 8, 9 };

    if ( binary_search( vec, 8 ) )
    {
        std::cout << "8 in the array!\n";
    }
    else
    {
        std::cout << "8 not in the array!\n";
    }

    if ( binary_search( vec, 5 ) )
    {
        std::cout << "5 in the array!\n";
    }
    else
    {
        std::cout << "5 not in the array!\n";
    }
}