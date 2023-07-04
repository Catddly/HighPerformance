#include <iostream>

// At the end of this function, it recursively call factorial().
// The tail call must be a pure function call to trigger tail call optimization.
//
// The expanation of this function:
//
// factorial( 4 )
// -> 4 * factorial( 3 )
// -> 4 * 3 * factorial( 2 )
// -> 4 * 3 * 2 * factorial( 1 )
// -> 4 * 3 * 2 * 1
// -> return
// 
// You can see the call stack will keep growing until all recursive calls are expanded.
// If n is very big or in some other case in a for loop, this may cause stack overflow.
// Keep creating call stack will have penalty on performance, since we create stack frame every time we
// make a recursive call.
size_t factorial( size_t n )
{
	if ( n == 1 )
		return 1;
	return n * factorial( n - 1 );
}

//-------------------------------------------------------------------------

size_t factorial_inner( size_t n, size_t accum )
{
	if ( n == 1 )
		return accum;
	accum *= n;
	return factorial_inner( n - 1, accum );
}

// This is the tail call optimization version of factorial.
// It still use a recursive function call but in a different way.
// At the end of this function we return the result of a recursive function call,
// and compiler is smart enough to know that factorial_inner() is the tail call,
// thus compiler know that no new call stack can be made upon this tail call,
// so it can reuse the current create call stack to make another function call.
//
// This optimization can save the time to continuaously create call stack and
// eliminate the risk of stack overflow.
//
// Note: in the meaning of tail call, it must be a recursive call who return a value or a recursive call
// at the end of the control flow, or a function call in a loop, but this loop have to be at the end of
// the control flow.
//
// The expanation of this function:
//
// factorial_tail_call( 4 )
// -> factorial_inner( 4, 1 )
// -> factorial_inner( 3, 4 )
// -> factorial_inner( 2, 12 )
// -> factorial_inner( 1, 24 )
// -> return 24;
//
// You can see compiler can definitely know that factorial_inner() is a tail call of factorial_tail_call().
// So compiler will apply tail call optimization on it.
size_t factorial_tail_call( size_t n )
{
	if ( n == 0 )
		return 1;
	return factorial_inner( n, 1 );
}

//-------------------------------------------------------------------------

// So the key to trigger tail call optimization, it to let compiler notice that:
//
// 1. It is a tail call. (at the end of a function, return object of the function)
// 2. It uses the same call stack continuaously.

// Practical Example: See Chapter-12 AsyncTask.cpp.
// The use of std::coroutine_handle<> continuation_ in Promise Task<T>::Type;

void TailCallOptimization()
{
	auto res = factorial( 5 );
	std::cout << "Res = " << res << '\n';

	res = factorial_tail_call( 5 );
	std::cout << "Res = " << res << '\n';
}