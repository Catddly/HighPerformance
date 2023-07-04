#include <coroutine>
#include <iostream>
#include <thread>

// std::suspend_always consists of:
//   await_ready(): indicate it can be resume or not
//   await_suspend( coroutine_handle<> ): called when coroutine suspended
//   await_resume(): called when coroutine resumed

struct MySuspend
{
	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return true;
	}

	constexpr void await_suspend( std::coroutine_handle<> ) const noexcept
	{}
	void await_resume() const noexcept
	{
		std::cout << "Just resume!\n";
	}
};

class Resumable
{
	struct Promise
	{
		Resumable get_return_object()
		{
			using Handle = std::coroutine_handle<Promise>;
			return Resumable{ Handle::from_promise( *this ) };
		}
		auto initial_suspend()
		{
			return MySuspend{};
		}
		auto final_suspend() noexcept
		{
			return std::suspend_always{};
		}
		void return_void()
		{}
		void unhandled_exception()
		{
			std::terminate();
		}
	};

	std::coroutine_handle<Promise> h_;
	explicit Resumable( std::coroutine_handle<Promise> h ) : h_{ h }
	{}
public:
	using promise_type = Promise;
	Resumable( Resumable&& r ) noexcept : h_{ std::exchange( r.h_, {} ) }
	{}
	~Resumable()
	{
		if ( h_ )
		{
			h_.destroy();
		}
	}
	bool resume()
	{
		if ( !h_.done() )
		{
			h_.resume();
		}
		return !h_.done();
	}
};

auto coroutine() -> Resumable
{
	std::cout << "Hello ";
	co_await std::suspend_always{};
	std::cout << "World!\n";
}

using namespace std::chrono;

template <class Rep, class Period>
auto operator co_await( duration<Rep, Period> d )
{
	struct Awaitable
	{
		system_clock::duration d_;
		Awaitable( system_clock::duration d ) : d_( d )
		{}
		bool await_ready() const
		{
			return d_.count() <= 0;
		}
		void await_suspend( std::coroutine_handle<> h )
		{
			/* ... */
		}
		void await_resume()
		{}
	};

	return Awaitable{ d };
}

void Await()
{
	{
		using namespace std::chrono_literals;

		auto resumable = coroutine();

		std::this_thread::sleep_for( 1s );
		resumable.resume();
	}
	
	//{
	//	using namespace std::chrono_literals;

	//	std::cout << "just about to go to sleep...\n";
	//	co_await 10ms; // Calls operator co_await()
	//	std::cout << "resumed\n";
	//}
}