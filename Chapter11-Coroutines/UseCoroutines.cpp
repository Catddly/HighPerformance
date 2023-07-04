#include <coroutine>
#include <iostream>
#include <thread>

// Compared to ordinary functions, a coroutine also has the following restrictions:
// 
// - A coroutine cannot use variadic arguments like f( const char*... ).
// - A coroutine cannot return auto or a concept type : auto f().
// - A coroutine cannot be declared constexpr.
// - Constructors and destructors cannot be coroutines.
// - The main() function cannot be a coroutine.

// The Return Object is the type that the coroutine returns and
// is typically a general class template designed for some specific use case, for example,
// generators or asynchronous tasks. The caller interacts with the return object to resume
// the coroutine and to get values emitted from the coroutine.
// The return object usually delegates all its calls to the coroutine handle.

// The Coroutine Handle is a non - owning handle to the Coroutine State.
// Through the coroutine handle we can resume and destroy the coroutine state.

// The coroutine state is what I have previously referred to as the coroutine frame.
// The coroutine state also contains the Promise.

// The Resumable is the owner of the coroutine state.
class Resumable // The return object
{
	struct Promise // coroutine state
	{
		// We should not call these functions directly;
		// instead, the compiler inserts calls to the promise objects when it transforms a coroutine into machine code.
		Resumable get_return_object()
		{
			using Handle = std::coroutine_handle<Promise>;
			return Resumable{ Handle::from_promise( *this ) };
		}
		auto initial_suspend()
		{
			return std::suspend_always{};
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

		static void* operator new( std::size_t sz )
		{
			std::cout << "\ncustom new for size " << sz << '\n';
			return ::operator new( sz );
		}
		static void operator delete( void* ptr )
		{
			std::cout << "\ncustom delete called\n";
			::operator delete( ptr );
		}
	};    
	// Nested class, see below

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

auto coroutine() -> Resumable        // Initial suspend
{
	std::cout << "3 ";
	co_await std::suspend_always{};  // Suspend (explicit)
	std::cout << "5 ";
}                                    // Final suspend then return

auto coro_factory() // Create and return a coroutine
{             
	auto res = coroutine();
	return res;
}

struct Widget
{
	auto coroutine() -> Resumable // A member function 
	{       
		std::cout << i_++ << " ";         // Access data member
		co_await std::suspend_always{};
		std::cout << i_++ << " ";
	}
	int i_{};
};

void UseCoroutines()
{
	std::cout << "1 ";
	auto resumable = coroutine(); // Create coroutine state
	std::cout << "2 ";
	resumable.resume();           // Resume
	std::cout << "4 ";
	resumable.resume();           // Resume
	std::cout << "6 \n";
	
	// to get promise type by the compiler
	std::coroutine_traits<Resumable>::promise_type test = {};

	auto r = coro_factory();
	r.resume();                       // Resume from main
	auto t = std::jthread{ [ r = std::move( r ) ]() mutable
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 2s );
		r.resume();                   // Resume from thread
	} };

	// w must be alive before coro dies
	auto w = Widget{ 99 };
	auto coro = w.coroutine();
	coro.resume();
	coro.resume();

	// The important thing to note here is that the actual coroutine is a member function, namely the call operator operator()().
	auto lambda = [] ( int i ) -> Resumable
	{
		std::cout << i << "\n";
		co_return;              // Make it a coroutine
	};
	auto coro0 = lambda( 42 );   // Call, creates the coroutine frame
	coro0.resume();              // Outputs: 42

	auto coro1 = [ i = 0 ] () mutable -> Resumable
	{
		std::cout << i++ << "\n";
		co_await std::suspend_always{};
		std::cout << i++ << "\n";
	}( );               // Invoke lambda immediately
	coro1.resume();     // Undefined behavior! Function object
	coro1.resume();     // already destructed
}