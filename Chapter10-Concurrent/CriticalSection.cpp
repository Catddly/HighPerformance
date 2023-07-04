#include <mutex>
#include <thread>
#include <iostream>
#include <cassert>

auto counter = 0; // Warning! Global mutable variable
auto counter_mutex = std::mutex{}; // this is also a global mutable variable, but it is safe to use it in different threads.

void increment_counter( int n )
{
    for ( int i = 0; i < n; ++i )
    {
        // critical section
        auto lck = std::scoped_lock{ counter_mutex }; // this will cause heavy thread contention
        ++counter;
    }
}

struct Account
{
    Account( int b )
        : balance_{ b }
    {}

    int balance_{ 0 };
    std::mutex m_{};
};

void transfer_money( Account& from, Account& to, int amount )
{
    // In this case, we use std::unique_lock, which provides us with the possibility to defer the locking of the mutex
    // Then, we explicitly lock both mutexes at the same time by using the std::lock() function.

    auto lock1 = std::unique_lock<std::mutex>{ from.m_, std::defer_lock };
    auto lock2 = std::unique_lock<std::mutex>{ to.m_, std::defer_lock };

    // Lock both unique_locks at the same time (prevent deadlock)
    std::lock( lock1, lock2 );

    from.balance_ -= amount;
    to.balance_ += amount;
}

void CriticalSection()
{
    constexpr auto n = int{ 100'000'000 };
    {
        auto t1 = std::jthread{ increment_counter, n };
        auto t2 = std::jthread{ increment_counter, n };
    }

    std::cout << counter << '\n';
    // If we don't have a data race, this assert should hold:
    assert( counter == ( n * 2 ) );

    auto account0 = Account{ 546 };
    auto account1 = Account{ 123 };

    {
        auto t1 = std::jthread{ transfer_money, std::ref( account0 ), std::ref( account1 ), 100 };
        auto t2 = std::jthread{ transfer_money, std::ref( account1 ), std::ref( account0 ), 50 };
    }

    assert( account0.balance_ == ( 546 - 50 ) );
    assert( account1.balance_ == ( 123 + 50 ) );
}