#include <queue>
#include <condition_variable>
#include <mutex>
#include <iostream>

auto cv = std::condition_variable{};

auto q = std::queue<int>{};  // shared queue (shared memory)
auto mtx = std::mutex{};     // Protects the shared queue
constexpr int sentinel = -1; // Value to signal that we are done

void print_ints()
{
    auto i = 0;
    while ( i != sentinel )
    {
        {
            auto lock = std::unique_lock<std::mutex>{ mtx };
            while ( q.empty() )
            {
                cv.wait( lock ); // The lock is released while waiting, thread goes to sleep, 
            
                // we must wrap inside a while loop.
                // in scenario that there are many consumers, a thread can be wake up but have something to fetch inside the queue.
                // This phenomenon is called spurious wakeup.
            }

            // it is the same
            //cv.wait( lock, []
            //{
            //    return !q.empty();
            //} );

            i = q.front();
            q.pop();
        }

        if ( i != sentinel )
        {
            std::cout << "Got: " << i << " from " << std::this_thread::get_id() << '\n';
        }
    }
}

auto generate_ints()
{
    // the sentinel count should be the number of consumers
    for ( auto i : { 1, 2, 3, 4, 5, 6, 7, sentinel, sentinel } )
    {
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        {
            auto lock = std::scoped_lock{ mtx };
            q.push( i );
        }
        cv.notify_one(); // wake one thread up
    }
}

void ProducerAndConsumer()
{
    auto producer = std::jthread{ generate_ints };
    auto consumer0 = std::jthread{ print_ints };
    auto consumer1 = std::jthread{ print_ints };
}