#include "MainEntryHelper.h"

// Continuation: When we say co_await in our code, we express that we are waiting for something that may or may not be ready for us.
// If it's not ready, co_await suspends the currently executing coroutine and returns control back to its caller.
// When the asynchronous task has completed, it should transfer the control
// back to the coroutine originally waiting for the task to finish.
// From here on, I will typically refer to the awaiting function as the continuation.

// Manually Header
void Await();
void AsyncTask();
void AsyncTCP();
void TailCallOptimization();

int main()
{
	// Entry( Await );
	// Entry( AsyncTask );
	// Entry( AsyncTCP );
	Entry( TailCallOptimization );
}