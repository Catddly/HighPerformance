#include <future>
#include <iostream>

// futures and promises, which represent two sides of a value.
// The future is the receiving side of the valueand the promise is the returning side of the value.

auto divide( int a, int b, std::promise<int>& p )
{
    if ( b == 0 )
    {
        auto e = std::runtime_error{ "Divide by zero exception" };
        p.set_exception( std::make_exception_ptr( e ) );
    }
    else
    {
        const auto result = a / b;
        p.set_value( result );
    }
}

int divide_pure( int a, int b ) // No need to pass a promise ref here! 
{
    if ( b == 0 )
    {
        throw std::runtime_error{ "Divide by zero exception" };
    }
    return a / b;
}

void FutureAndPromise()
{
    auto p = std::promise<int>{};
    auto p1 = std::promise<int>{};
    std::thread( divide, 45, 5, std::ref( p ) ).detach();
    std::thread( divide, 10, 0, std::ref( p1 ) ).detach();

    auto f = p.get_future();
    auto f1 = p1.get_future();
    try
    {
        const auto& result = f.get();  // Blocks until ready 
        std::cout << "Result: " << result << '\n';

        const auto& result0 = f1.get(); // Blocks until ready 
        std::cout << "Result: " << result0 << '\n';
    }
    catch ( const std::exception& e )
    {
        std::cout << "Caught exception: " << e.what() << '\n';
    }

    {
        // std::packaged_task will manage all the settings for us.
        // std::packaged_task is itself a callable object that can be moved to the std::thread object we are creating. 
        auto task = std::packaged_task<decltype( divide_pure )>{ divide_pure };
        auto f = task.get_future();
        std::thread{ std::move( task ), 50, 5 }.detach();

        // The code below is unchanged from the previous example 
        try
        {
            const auto& result = f.get(); // Blocks until ready (may throw)
            std::cout << "Result: " << result << '\n';
        }
        catch ( const std::exception& e )
        {
            std::cout << "Caught exception: " << e.what() << '\n';
        }
    }

    {
        auto f = std::async( divide_pure, 40, 5 );

        // The code below is unchanged from the previous example 
        try
        {
            const auto& result = f.get(); // Blocks until ready (may throw)
            std::cout << "Result: " << result << '\n';
        }
        catch ( const std::exception& e )
        {
            std::cout << "Caught exception: " << e.what() << '\n';
        }
    }
}