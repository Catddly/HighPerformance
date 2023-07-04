#include <latch>
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <barrier>
#include <random>
#include <array>
#include <semaphore>
#include <optional>
#include <atomic>
#include <cassert>
#include <complex>

// The stack memory is unlikely to be paged out by the operating system,
// so it is usually enough to run some code that will generate page faults
// and thereby map the virtual stack memory to physical memory.
// This process is called prefaulting.

void prefault_stack()
{
	// We don't know the size of the stack
	constexpr auto stack_size = 500u * 1024u;
	// Make volatile to avoid optimization
	volatile unsigned char mem[stack_size];
	std::fill( std::begin( mem ), std::end( mem ), 0 );
}

auto mutex = std::mutex{};

auto do_work()
{
	std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	auto lck = std::scoped_lock{ mutex };
	std::cout << "Do my work! " << std::this_thread::get_id() << "\n";
}

//-------------------------------------------------------------------------

// thread safe random function
auto random_int( int min, int max )
{
	// One engine instance per thread
	static thread_local auto engine = std::default_random_engine{ std::random_device{}( ) };

	auto dist = std::uniform_int_distribution<>{ min, max };
	return dist( engine );
}

//-------------------------------------------------------------------------

class Server
{
public:
	void handle( int code )
	{
		sem_.acquire();
		// Restricted section begins here.
		// Handle at most 4 requests concurrently.
		do_handle( code );
		sem_.release();
	}
private:
	void do_handle( int code )
	{

	}
	
	// std::binary_semaphore = std::counting_semaphore<1>;
	// A binary semaphore is guaranteed to be implemented more efficiently than a counting semaphore with a higher maximum value.

	// In this case, the semaphore is initialized with a value of 4,
	// which means that at most four concurrent requests can be handled at the same time.

	// Another important property of semaphores is that the thread that releases a semaphore may not be the thread that acquired it.
	std::counting_semaphore<4> sem_{ 4 };
};

//-------------------------------------------------------------------------

// A semaphore blocks a thread that tries to decrease the semaphore when it is already zero.
// A semaphore signals the blocked threads whenever a semaphore that has the value zero increments.

template <class T, int N>
class BoundedBuffer
{
	std::array<T, N> buf_;
	std::size_t read_pos_{};
	std::size_t write_pos_{};
	std::mutex m_;
	std::counting_semaphore<N> n_empty_slots_{ N };
	std::counting_semaphore<N> n_full_slots_{ 0 };

	void do_push( auto&& item )
	{
		n_empty_slots_.acquire();

		try
		{
			auto lock = std::unique_lock{ m_ };
			buf_[write_pos_] = std::forward<decltype( item )>( item );
			write_pos_ = ( write_pos_ + 1 ) % N;
		}
		catch ( ... )
		{
			n_empty_slots_.release();
			throw;
		}

		// Increment and signal that there is one more full slot
		n_full_slots_.release();
	}
public:
	void push( const T& item )
	{
		do_push( item );
	}
	void push( T&& item )
	{
		do_push( std::move( item ) );
	}
	auto pop()
	{
		// Take one of the full slots (might block)
		n_full_slots_.acquire();

		auto item = std::optional<T>{};
		try
		{
			auto lock = std::unique_lock{ m_ };
			item = std::move( buf_[read_pos_] );
			read_pos_ = ( read_pos_ + 1 ) % N;
		}
		catch ( ... )
		{
			n_full_slots_.release();
			throw;
		}

		// Increment and signal that there is one more empty slot
		n_empty_slots_.release();
		return std::move( *item );
	}
};

//-------------------------------------------------------------------------

// The variable might have changed from state A to state Band then back to state A without notifying the waiting thread.
// This is a phenomenon in lock - free programming called the ABA problem.

// No a production ready code
class SimpleMutex
{
	std::atomic_flag is_locked_{}; // Cleared by default
public:
	auto lock() noexcept
	{
		// If test_and_set() returns false,
		// it means that the caller managed to acquire the lock (setting the flag when it was previously cleared).
		// test() doesn't invalidate the cache line, whereas test_and_set() does.
		// This locking protocol is called test and test-and-set.
		while ( is_locked_.test_and_set() )
		{
			// while ( is_locked_.test() ); // Spin here

			// Do _NOT_ spin, wait (save some CPU resources)
			is_locked_.wait( true );
		}
	}
	auto unlock() noexcept
	{
		//is_locked_.clear();

		is_locked_.clear();
		is_locked_.notify_one(); // Notify blocked thread
	}
};

//-------------------------------------------------------------------------

// In addition, we can assume the following :
// 
// The Stats struct cannot be changed( maybe it's from a third-party library).
// We want the client to be unaware of the fact that our utility function flip_coin() is concurrent;
// that is, the concurrency of the flip_coin() function should be completely transparent to the caller.

struct Stats
{
	int heads_{};
	int tails_{};
};

std::ostream& operator<<( std::ostream& os, const Stats& s )
{
	os << "heads: " << s.heads_ << ", tails: " << s.tails_;
	return os;
}

void flip_coin( std::size_t n, Stats& outcomes )
{
	auto flip = [ &outcomes ] ( auto n )
	{
		auto heads = std::atomic_ref<int>{ outcomes.heads_ };
		auto tails = std::atomic_ref<int>{ outcomes.tails_ };
		for ( auto i = 0u; i < n; ++i )
		{
			random_int( 0, 1 ) == 0 ? ++heads : ++tails;
		}
	};
	auto t1 = std::jthread{ flip, n / 2 };         // First half
	auto t2 = std::jthread{ flip, n - ( n / 2 ) }; // The rest
}

//-------------------------------------------------------------------------

void AdditionalInCpp20()
{
	constexpr auto n_threads = 2;
	auto initialized = std::latch{ n_threads }; // no way to reset a latch
	auto threads = std::vector<std::thread>{};

	for ( auto i = 0; i < n_threads; ++i )
	{
		threads.emplace_back( [ & ]
		{
			prefault_stack(); // do the initialization job
			initialized.arrive_and_wait();
			do_work();
		} );
	}

	initialized.wait();
	std::cout << "Initialized, starting to work\n";

	for ( auto&& t : threads )
	{
		t.join();
	}

	//-------------------------------------------------------------------------

	// Barriers are similar to latches but with two major additions:
	// a barrier can be reused, and it can run a completion function whenever all threads have reached the barrier.

	// Whenever all threads have arrived (that is, when the internal counter of the barrier reaches zero) two things happens:
	//
	// - The completion function provided to the constructor is called by the barrier.
	// - The internal counter is reset to its initial value after the completion function has returned.
	// Barriers are useful in parallel programming algorithms that are based on the fork-join model.

	std::cout << "\n";

	{
		constexpr auto n = 5; // Number of dice

		auto done = false; // we check modified done in single thread, so this is safe.
		auto dice = std::array<int, n>{}; // no data race! thanks to barrier!
		auto threads = std::vector<std::thread>{};
		auto n_turns = 0;

		auto check_result = [ & ] () noexcept // Completion function
		{
			++n_turns;
			auto is_six = [] ( auto i )
			{
				return i == 6;
			};
			done = std::all_of( dice.begin(), dice.end(), is_six );
		};

		auto bar = std::barrier{ n, check_result };
		for ( int i = 0; i < n; ++i )
		{
			threads.emplace_back( [ &, i ]
			{
				while ( !done )
				{
					dice[i] = random_int( 1, 6 ); // Roll dice
					bar.arrive_and_wait();        // Join
				}
			} );
		}

		for ( auto&& t : threads )
		{
			t.join();
		}

		std::cout << n_turns << '\n';
	}

	//-------------------------------------------------------------------------

	{
		BoundedBuffer<int, 10> buf;

		auto producer = [] ( BoundedBuffer<int, 10>& buf )
		{
			for ( int i = 0; i < 30; ++i )
			{
				buf.push( i );
				std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
			}
		};

		auto consumer = [] ( BoundedBuffer<int, 10>& buf )
		{
			for ( int i = 0; i < 10; ++i )
			{
				int item = buf.pop();
				std::cout << "Consumer receive: " << item << "\n";
				std::this_thread::sleep_for( std::chrono::milliseconds( 400 ) );
			}
		};

		auto producer0 = std::jthread{ producer, std::ref( buf ) };
		auto consumer0 = std::jthread{ consumer, std::ref( buf ) };
		auto consumer1 = std::jthread{ consumer, std::ref( buf ) };
		auto consumer2 = std::jthread{ consumer, std::ref( buf ) };
	}

	//-------------------------------------------------------------------------

	// So, if a class contains virtual functions, pointers to dynamic memory,
	// and so on, it's no longer possible to just copy the raw bits of the object and expect it to work,
	// and hence it is not trivially copyable.

	{
		struct Point
		{
			int x_{};
			int y_{};
			// uncomment this will make Point _NOT_ lock-free anymore.
			//int z_{};
			//int w_{};
		};

		// On modern platforms, any std::atomic<T> where T fits into the native word size will typically be always lock - free.
		auto p = std::atomic<Point>{};       // OK: Point is trivially copyable
		//auto s = std::atomic<std::string>{}; // Error: Not trivially copyable


		assert( p.is_lock_free() );          // Runtime assert
		static_assert( std::atomic<Point>::is_always_lock_free );
		//static_assert( std::atomic<std::complex<double>>::is_always_lock_free );
	}

	// An atomic type that is guaranteed to always be lock - free is std::atomic_flag (regardless of the target platform).
	
	//-------------------------------------------------------------------------
	
	std::cout << "\n";

	// If you are mutating variables in methods that appear to be semantically read-only (const) from the client's perspective,
	// you should make the mutating variables thread-safe.

	{
		// specialization in C++20
		// this is a exception because std::shared_ptr<int> is _NOT_ trivially copyable.
		auto p = std::atomic<std::shared_ptr<int>>{};
		static_assert( !std::atomic<std::shared_ptr<int>>::is_always_lock_free ); // _NOT_ lock free

		// Thread T1 calls this function
		auto f1 = [&]
		{
			auto new_p = std::make_shared<int>( std::rand() ); // ... 
			p.store( new_p );
		};

		// Thread T2 calls this function
		auto f2 = [&]
		{
			auto local_p = p.load();
			// Use local_p... 
		};

		// It's not possible to write std::atomic<T&>;
		// instead, the standard library provides us with a template called std::atomic_ref. (in C++ 20)

		auto outcomes = Stats{};
		flip_coin( 30, outcomes );
		flip_coin( 10, outcomes );

		std::cout << outcomes << '\n';
		assert( outcomes.heads_ + outcomes.tails_ == 40 );
	}

	// C++20 introduced:

	// - The specialization std::atomic<std::shared_ptr<T>>.
	// - tomic references; that is, the std::atomic_ref<T> template.
	// - he waitand notify API, which is a lightweight alternative to using condition variables.
}