#include <vector>
#include <array>
#include <iostream>
#include <chrono>

#include "ScopeTimer.h"

// PallelArray is to trun AoS(Array of structure) to SoA(Structure of arrays)!
// Pros:
//
// 1. More efficient!
//
// Cons:
// 
// 1. May break encapsulation and the data security.
// 2. More cumbersome to ensure that the arrays are in sync.
// 3. When accessing multiple fields in the different array, it might slower than accessing all in a big array.
//

struct SmallObject
{
	std::array<char, 4> data_{};
	int score_{ std::rand() };
};

struct BigObject
{
	std::array<char, 256> data_{};
	int score_{ std::rand() };
};

template <class T>
auto sum_scores( const std::vector<T>& objects )
{
	ScopedTimer t{ "sum_scores" };

	auto sum = 0;
	for ( const auto& obj : objects )
	{
		sum += obj.score_;
	}
	return sum;
}

// A common example in some online game regiatration system
struct User
{
    std::string name_; // this is frequently used!
    std::string username_;
    std::string password_;
    std::string security_question_; // this is NOT frequently used!
    std::string security_answer_; // this is NOT frequently used!
    short level_{};
    bool is_playing_{};
};

struct AuthInfo
{
    std::string username_;
    std::string password_;
    std::string security_question_;
    std::string security_answer_;
};

// Smaller User Info
struct SUser
{
    std::string name_;
    std::unique_ptr<AuthInfo> auth_info_;
    short level_{};
    bool is_playing_{};
};

template <class User>
auto num_users_at_level( const std::vector<User>& users, short level )
{
    ScopedTimer t{ "num_users_at_level (using 128 bytes User)" };

    auto num_users = 0;
    for ( const auto& user : users )
        if ( user.level_ == level )
            ++num_users;
    return num_users;
}

template <class User>
auto num_playing_users( const std::vector<User>& users )
{
    ScopedTimer t{ "num_playing_users (using 128 bytes User)" };

    return std::count_if( users.begin(), users.end(),
                          [] ( const auto& user )
    {
        return user.is_playing_;
    } );
}

auto num_users_at_level_parallel( const std::vector<short>& users, short level )
{
    ScopedTimer t{ "num_users_at_level using short vector" };
    return std::count( users.begin(), users.end(), level );
}

auto num_playing_users_parallel( const std::vector<bool>& users )
{
    ScopedTimer t{ "num_playing_users using vector<bool>" };
    return std::count( users.begin(), users.end(), true );
}

void ParallelArray()
{
	std::cout << sizeof( SmallObject ) << '\n'; // Possible output is 8
	std::cout << sizeof( BigObject ) << '\n';   // Possible output is 260

	auto small_objects = std::vector<SmallObject>( 1'000'000 );
	auto big_objects = std::vector<BigObject>( 1'000'000 );

	// we want to sum the score of all objects
    auto score = sum_scores( small_objects );
    std::cout << "Small object sum score: " << score << '\n';
    score = sum_scores( big_objects );
    std::cout << "Large object sum score: " << score << '\n';

    auto users = std::vector<User>( 1'000'000 );
    auto res0 = num_users_at_level( users, 0 );
    std::cout << "Users At Level 0: " << res0 << '\n';
    auto res1 = num_playing_users( users );
    std::cout << "Count of playing users: " << res1 << '\n';

    std::cout << "\n------After Small User Optimization------\n\n";

    auto susers = std::vector<SUser>( 1'000'000 );
    auto res2 = num_users_at_level( susers, 0 );
    std::cout << "SUsers At Level 0: " << res2 << '\n';
    auto res3 = num_playing_users( susers );
    std::cout << "Count of playing susers: " << res3 << '\n';

    std::cout << "\n------Use Parallel Array------\n\n";

    auto levels = std::vector<short>( 1'000'000 );
    auto playing_users = std::vector<bool>( 1'000'000 );

    auto res4 = num_users_at_level_parallel( levels, 0 );
    std::cout << "Users At Level 0: " << res4 << '\n';
    auto res5 = num_playing_users_parallel( playing_users );
    std::cout << "Count of playing susers: " << res5 << '\n';
}