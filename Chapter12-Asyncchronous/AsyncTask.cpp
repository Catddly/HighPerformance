#include <coroutine>
#include <variant>
#include <type_traits>
#include <semaphore>
#include <cassert>
#include <iostream>

#include <boost/asio.hpp>

namespace asio = boost::asio;

// Note: The standard library already provides a type that allows a function
// to return an object that a caller can use for waiting on a result to be computed, namely std::future.
// We could potentially wrap std::future into something
// that would conform to the awaitable interface.
// However, std::future does not support continuations (no blocking),
// which means that whenever we try to get the value from a std::future,
// we block the current thread. In other words,
// there is no way to compose asynchronous operations without blocking when using std::future.

// A coroutine that uses co_return value; must have a promise type that implements return_value().
// However, coroutines that use co_return;, or run off the body without returning a value,
// must have a promise type that implements return_void().
// Implementing a promise type that contains both return_void() and return_value() generates a compilation error.

template <typename T>
class [[nodiscard]] Task
{
    struct Promise
    {
        // use std::monostate to make variant default constructible
        std::variant<std::monostate, T, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;  // A waiting coroutine

        auto get_return_object() noexcept
        {
            return Task{ *this };
        }
        void return_value( T value )
        {
            result_.template emplace<1>( std::move( value ) );
        }
        void unhandled_exception() noexcept
        {
            result_.template emplace<2>( std::current_exception() );
        }
        auto initial_suspend()
        {
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            struct Awaitable
            {
                bool await_ready() noexcept
                {
                    return false; // always suspend
                }
                auto await_suspend( std::coroutine_handle<Promise> h ) noexcept
                {
                    return h.promise().continuation_; // transfer control to the waiting coroutine
                }
                void await_resume() noexcept
                {}
            };

            return Awaitable{};
        }
    };

    std::coroutine_handle<Promise> h_;
    explicit Task( Promise& p ) noexcept
        : h_{ std::coroutine_handle<Promise>::from_promise( p ) }
    {}
public:
    using promise_type = Promise;
    Task( Task&& t ) noexcept : h_{ std::exchange( t.h_, {} ) }
    {}
    ~Task()
    {
        if ( h_ ) h_.destroy();
    }

    // Awaitable interface
    bool await_ready()
    {
        return false;
    }
    auto await_suspend( std::coroutine_handle<> c )
    {
        // Use Symmetric transfer to avoid stack overflow
        // 
        // An optimization called tail call optimization is then guaranteed to happen by the compiler.
        // 
        // If the Promise object associated with coroutine a() directly called resume() on the handle to coroutine b(),
        // a new call frame to resume b() would be created on the stack on top of the call frame for a().
        // This process would be repeated over and over again in the loop, creating new nested call frames on the stack for each iteration.
        // This call sequence when two functions call each other is a form of recursion, sometimes called mutual recursion.
        // 
        // Example:
        // 
        // auto a() -> Task<int> { co_return 42; }
        // auto b( int n ) -> Task<int>
        // {
        //     auto sum = 0;
        //     for (auto i = 0; i < n; ++i)
        //     {
        //         // every call will open a coroutine callframe of a()!
        //         sum += co_await a();
        //     }
        //     co_return sum;
        // }
        // 
        // In our case, this means that the compiler will be able to transfer control directly
        // to the continuation without creating a new nested call frame.
        // Use to break the chain of coroutines.

        // Symmetric transfer
        h_.promise().continuation_ = c; // save the suspend handle
        return h_;
    }
    auto await_resume() -> T
    {
        auto& result = h_.promise().result_;
        if ( result.index() == 1 )
        {
            return std::get<1>( std::move( result ) );
        }
        else
        {
            std::rethrow_exception( std::get<2>( std::move( result ) ) );
        }
    }
};

template <>
class [[nodiscard]] Task<void>
{
    struct Promise
    {
        std::exception_ptr e_; // No std::variant, only exception
        std::coroutine_handle<> continuation_;
        auto get_return_object() noexcept
        {
            return Task{ *this };
        }
        void return_void()
        {}   // Instead of return_value() 
        void unhandled_exception() noexcept
        {
            e_ = std::current_exception();
        }
        auto initial_suspend()
        {
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            struct Awaitable
            {
                bool await_ready() noexcept
                {
                    return false;
                }
                auto await_suspend( std::coroutine_handle<Promise> h ) noexcept
                {
                    return h.promise().continuation_;
                }
                void await_resume() noexcept
                {}
            };
            return Awaitable{};
        }
    };
    std::coroutine_handle<Promise> h_;
    explicit Task( Promise& p ) noexcept
        : h_{ std::coroutine_handle<Promise>::from_promise( p ) }
    {}
public:
    using promise_type = Promise;

    Task( Task&& t ) noexcept : h_{ std::exchange( t.h_, {} ) }
    {}
    ~Task()
    {
        if ( h_ ) h_.destroy();
    }
    // Awaitable interface
    bool await_ready()
    {
        return false;
    }
    auto await_suspend( std::coroutine_handle<> c )
    {
        h_.promise().continuation_ = c;
        return h_;
    }
    void await_resume()
    {
        if ( h_.promise().e_ )
            std::rethrow_exception( h_.promise().e_ );
    }
};

template<typename T>
using Result = decltype( std::declval<T&>().await_resume() );

namespace detail
{
    template <typename T>
    class SyncWaitTask // A helper class only used by sync_wait()
    {
        struct Promise
        {
            T* value_{ nullptr };
            std::exception_ptr error_{ nullptr };
            std::binary_semaphore semaphore_{ 0 };

            SyncWaitTask get_return_object() noexcept
            {
                return SyncWaitTask{ *this };
            }
            void unhandled_exception() noexcept
            {
                error_ = std::current_exception();
            }
            auto yield_value( T&& x ) noexcept // Result has arrived
            {
                value_ = std::addressof( x );
                return final_suspend(); // call final_suspend immediatelly, instead of called passively
            }
            auto initial_suspend() noexcept
            {
                return std::suspend_always{};
            }
            auto final_suspend() noexcept
            {
                // the job of this Awaitable class is just to signal the semaphore that
                // we have finish all the instrcutions in the coroutine, semaphore can be signaled!
                struct Awaitable
                {
                    bool await_ready() noexcept
                    {
                        return false;
                    }
                    void await_suspend( std::coroutine_handle<Promise> h ) noexcept
                    {
                        h.promise().semaphore_.release(); // Signal! 
                    }
                    void await_resume() noexcept
                    {}
                };

                return Awaitable{};
            }
            void return_void() noexcept
            {
                assert( false );
            }
        };

        std::coroutine_handle<Promise> h_;
        explicit SyncWaitTask( Promise& p ) noexcept
            : h_{ std::coroutine_handle<Promise>::from_promise( p ) }
        {}
    public:
        using promise_type = Promise;

        SyncWaitTask( SyncWaitTask&& t ) noexcept
            : h_{ std::exchange( t.h_, {} ) }
        {}
        ~SyncWaitTask()
        {
            if ( h_ ) h_.destroy();
        }

        // Called from sync_wait(). Will block and retrieve the
        // value or error from the task passed to sync_wait()
        // This can be called on different thread.
        T&& get()
        {
            auto& p = h_.promise();
            h_.resume();
            p.semaphore_.acquire(); // Block until signal
            if ( p.error_ )
                std::rethrow_exception( p.error_ );
            return static_cast<T&&>( *p.value_ );
        }
        // No awaitable interface, this class will not be co_await:ed
    };
}

template <typename T>
Result<T> sync_wait( T&& task )
{
    if constexpr ( std::is_void_v<Result<T>> )
    {
        struct Empty
        {};

        auto coro = [ & ] () -> detail::SyncWaitTask<Empty>
        {
            co_await std::forward<T>( task );
            co_yield Empty{};
            // upon is transfered by the compiler to
            // co_await promise.yield_value(some_value); 
            // UNREACHABLE_CODE
            assert( false );
        };
        coro().get();
    }
    else
    {
        auto coro = [ & ] () -> detail::SyncWaitTask<Result<T>>
        {
            // co_yield the return value of async task
            co_yield co_await std::forward<T>( task );
            // This coroutine will be destroyed before it
            // has a chance to return.
            // UNREACHABLE_CODE
            assert( false );
        };
        return coro().get();
    }
}

auto height() -> Task<int>
{
    co_return 20;
}

auto width() -> Task<int>
{
    co_return 30;
}

auto area() -> Task<int>
{
    co_return 
        co_await height() * 
        co_await width();
}

//-------------------------------------------------------------------------

// This code is here just to get our example up and running
struct Detached
{
    struct promise_type
    {
        auto get_return_object()
        {
            return Detached{};
        }
        auto initial_suspend()
        {
            return std::suspend_never{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_never{};
        }
        void unhandled_exception()
        {
            std::terminate();
        } // Ignore
        void return_void()
        {}
    };
};

// Starting operations and detaching them should typically be avoided.
// This is just a example.

Detached run_task_impl( asio::io_context& ctx, Task<void>&& t )
{
    auto wg = asio::executor_work_guard<asio::io_context::executor_type>{ ctx.get_executor() };
    co_await t;
}

void run_task( asio::io_context& ctx, Task<void>&& t )
{
    run_task_impl( ctx, std::move( t ) );
    ctx.run();
}

template <typename R, typename P>
auto async_sleep( asio::io_context& ctx,
                  std::chrono::duration<R, P> d )
{
    struct Awaitable
    {
        asio::system_timer t_;
        std::chrono::duration<R, P> d_;
        boost::system::error_code ec_{};

        bool await_ready()
        {
            return d_.count() <= 0;
        }
        void await_suspend( std::coroutine_handle<> h )
        {
            t_.expires_from_now( d_ );
            // use callback here
            t_.async_wait( [ this, h ] ( auto ec ) mutable
            {
                this->ec_ = ec;
                h.resume();
            } );
        }
        void await_resume()
        {
            if ( ec_ )
                throw boost::system::system_error( ec_ );
        }
    };

    return Awaitable{ asio::system_timer{ctx}, d };
}

auto test_sleep( asio::io_context& ctx ) -> Task<void>
{
    using namespace std::chrono_literals;

    std::cout << "Hello!  ";
    co_await async_sleep( ctx, 500ms );
    std::cout << "Delayed output\n";
}

void AsyncTask()
{
    auto a = area();
    int value = sync_wait( a );

    std::cout << value << "\n";

    //-------------------------------------------------------------------------

    using namespace std::chrono_literals;

    // boost.asio without C++ coroutine (using callback)
    {
        auto ctx = asio::io_context{};
        auto timer = asio::system_timer{ ctx };
        timer.expires_from_now( 1000ms );
        timer.async_wait( [] ( auto error ) // Callback
        {
            // Ignore errors..                          
            std::cout << "Hello from delayed callback\n";
        } );
        std::cout << "Hello from main\n";
        ctx.run();
    }

    std::cout << "\n";

    // boost.asio using C++20 coroutine
    {
        auto ctx = asio::io_context{};
        run_task( ctx, test_sleep( ctx ) );
    }
}