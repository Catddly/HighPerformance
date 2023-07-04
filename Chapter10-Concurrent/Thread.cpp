#include <iostream>
#include <thread>
#include <new>

// Common thread model:
// 
// - CPU contains HW thread (hardware thread)
//   Those are the execution units on the CPU. 
//   Some cores can execute two hardware threads. This is usually called hyperthreading.
// - OS contains OS thread (operation system thread)
//   The operating system scheduler determines when and
//   for how long an operating system thread is executed by a hardware thread.
// - Inside program we have std::thread.
//   A std::thread object is nothing more than an ordinary C++ object that
//   may or may not be associated with an underlying operating system thread.
//   Two instances of std::thread cannot be associated with the same underlying thread.

// A thread is not joinable if it has been:
// 
// - Default constructed; that is, if it has nothing to execute.
// - Moved from( its associated running thread has been transferred to another std::thread object ).
// - Detached by a call to detach().
// - Already joined by a call to join().

void print()
{
	std::this_thread::sleep_for( std::chrono::milliseconds{ 150 } );
	std::cout << "Thread ID: " << std::this_thread::get_id() << '\n';
}

void print_with_stop_token( std::stop_token stoken )
{
	while ( !stoken.stop_requested() )
	{
		std::cout << std::this_thread::get_id() << '\n';
		std::this_thread::sleep_for( std::chrono::milliseconds{ 150 } );
	}
	std::cout << "Stop requested\n";
}

void Thread()
{
	auto t1 = std::thread{ print };

	std::cout << std::thread::hardware_concurrency() << '\n'; // check HW thread count
	std::cout << std::boolalpha << t1.joinable() << '\n';

	t1.join();
	std::cout << "Thread ID: " << std::this_thread::get_id() << '\n';

	auto jt1 = std::jthread{ print_with_stop_token }; // C++20
	std::cout << "main thread: goes to sleep\n";
	std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
	std::cout << "main thread: request jthread to stop\n";

	jt1.request_stop();

	std::cout << "Hardware destructive interference size " << std::hardware_destructive_interference_size << " bytes.\n";

	// each element will now fall on different cache line.
	struct alignas( std::hardware_destructive_interference_size ) Element
	{
		int counter_{};
	};

	//jt1.join(); // the destructor of std::jthread will call join() for us.

	// std::jthread in special compare with std::thread
	// 
	// - std::jthread has support for stopping a thread using a stop token.
	//   This is something that we had to implement manually before C++20 when using std::thread.
	// - Instead of terminating the app when it is being destructed in a non-joinable state,
	//   the destructor of std::jthread will send a stop request and join the thread on destruction.

	// if before std::terminate() is called, std::thread haven't call join() or detach() yet, std::abort() will be called.
}