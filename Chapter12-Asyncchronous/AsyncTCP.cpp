#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

using namespace std::chrono;
namespace asio = boost::asio;
using boost::asio::ip::tcp;

auto serve_client( tcp::socket socket ) -> asio::awaitable<void>
{
    std::cout << "New client connected\n";
    auto ex = co_await asio::this_coro::executor;
    auto timer = asio::system_timer{ ex };
    auto counter = 0;
    while ( true )
    {
        try
        {
            auto s = std::to_string( counter ) + "\n";
            auto buf = asio::buffer( s.data(), s.size() );
            auto n = co_await async_write( socket, buf, asio::use_awaitable );
            std::cout << "Wrote " << n << " byte(s)\n";
            ++counter;
            timer.expires_from_now( 100ms );
            co_await timer.async_wait( asio::use_awaitable );
        }
        catch ( ... )
        {
            // Error or client disconnected
            break;
        }
    }
}

auto listen( tcp::endpoint endpoint ) -> asio::awaitable<void>
{
    // The executor is the object responsible for actually executing our asynchronous functions.
    // An executor may represent a thread pool or a single system thread, for example.
    auto ex = co_await asio::this_coro::executor;
    auto a = tcp::acceptor{ ex, endpoint };

    while ( true )
    {
        auto socket = co_await a.async_accept( asio::use_awaitable );
        auto session = [ s = std::move( socket ) ] () mutable
        {
            auto awaitable = serve_client( std::move( s ) );
            return awaitable;
        };
        asio::co_spawn( ex, std::move( session ), asio::detached );
    }
}

// By using coroutine, we can execute all this on single thread.
// No need for synchronization primitives!

// The coroutines in this example are all executed on the same thread,
// which makes the locking of shared resources unnecessary.
// Imagine we had a global counter that each session updated.
// If we used multiple threads, the access to the global counter would need some kind of synchronization (using a mutex or an atomic data type).
// This is not necessary for coroutines that execute on the same thread.
// In other words, coroutines that execute on the same thread can share state without using any locking primitives.

void AsyncTCP()
{
    auto server = []
    {
        // it defines a TCP endpoint (with port 37259) and starts listening for incoming client connections on the endpoint.
        auto endpoint = tcp::endpoint{ tcp::v4(), 37259 };
        auto awaitable = listen( endpoint );
        return awaitable;
    };

    auto ctx = asio::io_context{};

    // The function boost::asio::co_spawn() starts a detached concurrent flow.
    asio::co_spawn( ctx, server, asio::detached );

    // Creating detached tasks using boost::asio::co_spawn(),
    // as shown in the example, should be done with utmost caution.
    // A fairly new programming paradigm for avoiding detached work is called structured concurrency.
    // 
    // It aims to solve exception safety and the cancellation of multiple asynchronous tasks
    // by encapsulating concurrency into general and reusable algorithms such as when_all() and stop_when().
    // The key idea is to never allow some child task to exceed the lifetime of its parent.
    // This makes it possible to pass local variables by reference to asynchronous child operations safely and with better performance.
    // Strictly nested lifetimes of concurrent tasks also make the code easier to reason about.

    ctx.run(); // Run event loop from main thread
}