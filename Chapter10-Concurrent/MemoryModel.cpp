#include <iostream>
#include <array>
#include <optional>

// The memory model is closely related to concurrency since
// it defines how the reads and writes to the memory should be visible among threads.
// This is a rather complicated subject that touches on both compiler optimizations and
// multicore computer architecture.

// The good news, though, is that if your program is free from data races and
// you use the memory order that the atomics library provides by default,
// your concurrent program will behave according to an intuitive memory model that is easy to understand.

// Instruction reordering: When we write and run a program,
// it would be reasonable to assume that the instructions in the source code will be executed
// in the same order as they appear in the source code.
// This is _NOT_ true.
// Both the compiler and the hardware will reorder instructions with the goal of executing the program more efficiently.
// The compiler (and hardware) are free to reorder instructions as long as the reordering is not observable when running the program.

// Note from https://www.youtube.com/watch?v=A8eCGOqgvH4&list=WL&index=2
// There is a transformation from source code to actual execution:
// transformation = reorder + invent + remove
// including:
// - Compiler: subexpression elimination, register allocation(to save time writing a variable in a loop, ...), STM...
// - Processor: prefetch, speculation (e.g. branch prediction: if it is right, we save time; if it is wrong, we undo),
//              overlap writes, OoO (Out of Order execution), instruction pipelining, HTM...
// - Cache: store buffers (store writes, because writes is mush more expensive that reads,
//                         and flush asynchronously to main memory, instead of waiting it to load to main memory and do nothing,
//                         if some variable read after write, processor will load it from store buffers(faster) ),
//          private/shared cache...
//
// Reorder can happen in SC (sequencial consistency) compiler and processor, such as store buffer.
// But programmer do _NOT_ care about there things, we only want the program executes as we want to see.
// Goal: Try to maintain this illusion.

// We want to do less memory synchronazation as possible.
// 80% of the computer works are just moving data.

// Dekker's and Peterson's Algorithm

// SC-DRF (SC Data-race-free)

// In a transaction region (critical section), it is always unsafe to move code out of the region, but safe to move it into the region.
// the code under the release (unlock) can move itself as high as it just under the acquire (lock), same for the code at the top of acquire.
// 
// Example:
// i1
// fence.acquire() ↓↓↓↓↓
// i2
// i3
// fence.release() ↑↑↑↑↑
// i4
//
// It is safe to move instructions like this:
// fence.acquire() ↓↓↓↓↓
// i4
// i2
// i3
// i1
// fence.release() ↑↑↑↑↑ (release here not only ensure the memory use in critical section, but all the operations happened in this thread.
//                        so you can _NOT_ move instructions below it to being above the acquire)
// this memory ordering gives us a one-way barrier (an acquire-release barrier)
// More precisely: A release store makes its prior accesses visible to a thread performing an acquire load that sees (pairs with) that store.

// Plain Acq/Rel
// 
// release ( mutex 1 )
//
// acquire ( mutex 2 )
//
// It is safe to move release and acquire like this, since they are different mutex.
// 
// acquire ( mutex 2 )
// release ( mutex 1 )
//

// SC Acq/Rel
// 
// release ( mutex 1 )
//
// acquire ( mutex 2 )
//
// It is safe to move release and acquire like this, since they are different mutex.
// 
// This can _NOT_ be true, acquire and release can _NOT_ be reorder to have overlap region as if the programmer sees.
// acquire ( mutex 2 )
// release ( mutex 1 )
//

// Bandwidth * Latency = Concurrency

// Transitivity: here x and y are all std::atomic, but g is just a global variable. All variables are initially zero.
// The assertion below must be TRUE! (SC guarantee)
//
// Therad 1:           Thread 2:           Thread 3:
// 
// g = 1;              if (x == 1)         if (y == 1)
// x = 1;                  y = 1;              assert( g == 1 );
//
// Total store order: here x and y are all std::atomic. All variables are initially zero.
// Only one of the printf should be executed!! (SC guarantee)
//
// Therad 1:           Thread 2:           Thread 3:                                    Thread 4:
// 
// x = 1;              y = 1;              if (y == 1 && x == 0)                        if (y == 0 && x == 1)
//                                             printf("Thread 2 comes first!");             printf("Thread 1 comes first!");
//

// compare_exchange_ (weak or strong)
// weak allows sprious failures.
// When you decide to use CAS (compare and swap) loops, use weak.
// When you want to a single test, use strong.

// load (acquire) <--> store (release)

// Why standalone fences are suboptimal?
// They are more heavyweight synchronazation and less optimal.

// Store is more expensive than load.
// - Stores do more work.
// - Loads outnumber stores.
//
// The overhead of load when using a atomic should just be a little more
// expensive than ordinary load.

// Full-SC
//
//
//
// SC-DRF
//
//
//
// Common Single Thread

// Without relaxed atomics, 'simultaneously' really means what you thought it did.

// memory_order_
// relaxed: allow atomic relative instructions to flow around. (i.e. no order guarantee) No operation orders memory.
// acquire:
// release:
// acq_rel: acquire release which is no SC. (Plain Acquire-Release)
// seq_cst: default. Most strong memory order. (SC Acquire-Release)
//
// acquire, acq_rel, seq_cst: a load operation performs an acquire operation on the affected memory location.
// release, acq_rel, seq_cst: a store operation performs an release operation on the affected memory location.
//
// When you use atomic::load( memory_order order = memory_order_seq_cst ),
// You shall not pass memory_order_release or memory_order_acq_rel.
// Vice versa.
//
//
// Example to use relexed atomic judiciously:
//
// - Event Counters.
// - Dirty Flag.
// - Refernce Counting.
//
// If a atomic is not shared at all or the program guarantee to have other synchronizations in other way,
// it is ok to use relexed atomics.
// 
// Wrap the relexed atomic object in some class to protect atomic ops to spread out to callers (users).
//

// Inside MM (memory model):
// - mutexes
// - atomics
// - memory barriers
// - acquire/release

// Outside MM:
//
// keyword 'volatile'
//
// This is used to communicate with compiler when you are dealing with things outside of C/C++ MM.

template <class T, size_t N>
class LockFreeQueue
{
	std::array<T, N> buffer_{};     // Used by both threads
	std::atomic<size_t> size_{ 0 }; // Used by both threads
	size_t read_pos_{ 0 };          // Used by reader thread
	size_t write_pos_{ 0 };         // Used by writer thread
	static_assert( std::atomic<size_t>::is_always_lock_free );

	bool do_push( auto&& t ) // Helper function
	{
		if ( size_.load() == N )
		{
			return false;
		}

		buffer_[write_pos_] = std::forward<decltype( t )>( t );
		write_pos_ = ( write_pos_ + 1 ) % N;
		size_.fetch_add( 1 );
		return true;
	}
public:
	// Writer thread
	bool push( T&& t )
	{
		return do_push( std::move( t ) );
	}
	bool push( const T& t )
	{
		return do_push( t );
	}
	// Reader thread
	auto pop() -> std::optional<T>
	{
		auto val = std::optional<T>{};
		if ( size_.load() > 0 )
		{
			val = std::move( buffer_[read_pos_] );
			read_pos_ = ( read_pos_ + 1 ) % N;
			size_.fetch_sub( 1 );
		}
		return val;
	}
	// Both threads can call size()
	auto size() const noexcept
	{
		return size_.load();
	}
};

void MemoryModel()
{
	{
		int a = 10;      // 1 
		std::cout << a;  // 2 
		int b = a;       // 3 
		std::cout << b;  // 4 
		// Observed output: 1010
	}

	std::cout << "\n";

	{
		// we swap the instruction like this, and its output is the same
		int a = 10;      // 1 
		int b = a;       // 3 This line moved up  
		std::cout << a;  // 2 This line moved down 
		std::cout << b;  // 4 
		// Observed output: 1010 
	}

	{
		constexpr auto ksize = size_t{ 100 };
		using MatrixType = std::array<std::array<int, ksize>, ksize>;

		auto cache_thrashing = [] ( MatrixType& matrix, int v ) // 1 
		{
			for ( size_t i = 0; i < ksize; ++i )           // 2 
				for ( size_t j = 0; j < ksize; ++j )       // 3 
					matrix[j][i] = v;                      // 4 
		};

		// that code similar to this produces a lot of cache misses, which hurts performance.
		// A compiler is free to optimize this by reordering the for statements, like this:

		auto cache_thrashing_optimized = [] ( MatrixType& matrix, int v ) // 1 
		{
			for ( size_t j = 0; j < ksize; ++j )                          // 3 Line moved up 
				for ( size_t i = 0; i < ksize; ++i )                      // 2 Line moved down 
					matrix[j][i] = v;                                     // 4  
		};

		// Optimizations performed by the compilerand the hardware
		// ( including instruction pipelining, branch prediction, and cache hierarchies )
		// are very complicatedand constantly evolving technologies.

		// If you have been trying to debug an optimized build of your program,
		// you have probably noticed that it can be hard to step through it because of the re-orderings.
		// So, by using a debugger, the re-orderings are in some sense observable,
		// but they are not observable when running the program in a normal way.

		// The compiler (and hardware) does all its optimizations based on what is true and observable for one thread only.
		// It doesn't care about programmer use multi-threading or not.
		// The compiler cannot know what other threads are able to observe through shared variables,
		// so it is our job as programmers to inform the compiler of what re-orderings are allowed.

		// When protecting a critical section with a mutex,
		// it is guaranteed that only the thread that currently owns the lock can execute the critical section.
		// But, the mutex is also creating memory fences around the critical section to inform the system that
		// certain re-orderings are not allowed at the critical section boundaries.
		// When acquiring the lock, an acquire fence is added, and when releasing the lock, a release fence is added.

		// suppose we have four instructions: i1, i2, i3, i4.
		// both i2 and i3 access shared memory and need to be protect by critical section.
		// fence is one way, instruction can not pass the one way fence.
		// 
		// i1
		// fence.acquire() ↓↓↓↓↓
		// i2
		// i3
		// fence.release() ↑↑↑↑↑
		// i4
		// 
		// No read or write instructions can pass above the acquire fence, and nothing can pass below the release fence.
		// When acquiring a mutex, we are creating an acquire memory fence.
		// It tells the system that no memory accesses( reads or writes ) can be moved above the line where the acquire fence is located.
		// (i.e. read and write operations must be protected by critical section)
		// It is possible for the system to move the i4 instruction above the release fence beyond the i3 and i2 instructions,
		// but no further than that because of the acquire fence.

		// Now, let's have a look at atomic variables instead of mutexes.
		// When we use a shared atomic variable in our program, it gives us two things:

		// - Protection against torn writes: 
		//   The atomic variable is always updated atomically so there is no way a reader can read a partially written value.
		// - Synchronization of memory by adding sufficient memory fences:
		//   This prevents certain instruction re - orderings to guarantee a certain memory order specified by the atomic operations.
	}

	// Example to use relexed atomics:
	//-------------------------------------------------------------------------

	{
		//void launch_workers();
		//void join_workers();

		// Thread Func
		//while ( ... )
		//{
		//	if ( ... )
		//		counter.fetch_add( 1, memory_order_relexed );
		//}

		// Main Func
		//launch_workers();
		//
		//...
		//
		//join_workers();
		//
		//std::cout << counter;

		// In this example, counter is only use in thread function and after the joining of the thread,
		// it is guarantee by the launch-join pair that counter only get increment in that region,
		// so it is ok to let counter to be reordered.
		// It has no effect on our final result of counter, since the read of counter is below join_workers(),
		// after join_workers(), counter is guarantee to be used up.
		//
		// Better solution: wrap counter in a class call SyncEventCounter or ThreadSafeEventCounter to _NOT_ expose memory_order flags.
	}

	{
		// Thread Func
		//while ( !stop.load( memory_order_relaxed ) )
		//{
		//	if ( ... )
		//	{
		//		dirty.store(true, memory_order_release);
		//	}
		//}

		// Main Func
		//launch_workers();
		//stop = true // default as scq_cst
		//join_workers();
		//if ( dirty.load( memory_order_acquire ) )
		//	do_something();

		// Since we have work to do (memory ops) in do_something(), dirty should be memory_order_acquire/release.
		// Remember that 'acquire' flag will make sure the instructions below it can see the memory changes after the release.
		// By this way we can ensure the correct order of executing do_something() if dirty is set to true.
		// (But in this example this order is also ensure by the launch-join pairs, so dirty can be memory_order_relexed)
		//
		// stop.load can be set to relexed if setting stop doesn't publish data.
		// be careful when we setting stop, we use sequencial consist.
	}

	{
		// Thread 1 (in a copy ctor)
		//control_block_ptr = other->control_block_ptr;
		//control_block_ptr->refs.fetch_add(1, memory_order_relaxed);

		// Thread 2 (in a dtor)
		// 
		// use object... (A)
		// 
		//if ( control_block_ptr->refs.fetch_sub( 1, memory_order_acq_rel ) ) // here we have RAW (read after write), so use acq_rel
		//{
		//	delete control_block_ptr
		//                             (B)
		//}

		// In this reference counting example:
		// Increment can be relexed (not a publish operation). We only increse the counter and all jobs done.
		// 
		// Decrement can be acq_rel. Because delete operations depends on it and have side effects to other thread when they try to decrease the refs.
		// By ensuring this, the memory order is correct. (delete will only happened when refs is set to 0) (i.e. compiler or processor can _NOT_ move delete up against other fetch_sub).
		// we can _NOT_ use memory_order_release here, it can _NOT_ stop transformation to move line A below fetch_sub, thus cause a bug.
		// (we delete the control_block_ptr and we still using it)
	}

	// So, you see the key to decide whether a atomic can use a relexed memory order,
	// it's to check whether using this atomic have _SIDE EFFECT_ to others.
	// (i.e. whether the usage of atomic has publish some data or do some operations depends on others)
}