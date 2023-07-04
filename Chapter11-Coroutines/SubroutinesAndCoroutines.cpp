#include <iostream>

// To understand the difference between coroutines and subroutines (ordinary functions), 
// we will here focus on the most basic properties of subroutines and coroutines,
// namely, how to start, stop, pause, and resume them.

// A subroutine is started when some other part of our program calls it.
// When the subroutine returns back to the caller, the subroutine stops.
// The call chain of subroutines is strictly nested.

// Coroutines can also be started and stopped just like subroutines,
// but they can also be suspended (paused) and resumed.
// The point where a coroutine is suspended and resumed is called a suspend/resume point.
// The states of local variables inside a coroutine are preserved while the coroutine is suspended.

// To summarize, coroutines are subroutines that also can be suspended and resumed.
// Another way to look at it is to say that
// subroutines are a specialization of coroutines that cannot be suspended or resumed.

// Basic of CPU registers, instructions and call stack.
// The CPU keeps track of the address of the currently executing instruction in a register called a 'program counter'.
// Except for some general purpose registers, CPU have some dedicated registers:
// - Program counter (PC): The register that stores the memory address of the currently executing instruction.
//   This value is automatically incremented whenever an instruction is executed.
//   Sometimes it is also called an instruction pointer.
// - Stack pointer (SP): It stores the address of the top of the currently used call stack.
//   Allocating and deallocating stack memory is a matter of changing the value stored in this single register.

// The CPU can operate in two modes: 'user' mode or 'kernel' mode.
// When the CPU is executing in user mode, it runs with restricted privileges that cannot access hardware.
// The operating system provides system calls that run in kernel mode.
// A C++ library function such as std::puts(), which prints values to stdout,
// must therefore make a system call to complete its task,
// forcing the CPU to switch between user mode and kernel mode.
// But the CPU is a shared resource among all user processes and the operating system,
// and whenever we need to switch between tasks (for example, when entering kernel mode),
// the state of the processor, including all of its registers,
// needs to be saved in memory so that it can be resumed later on.

// Subroutines execution flow:
//
// - Calling and returning (jumping between points in the code).
// - Passing parameters—parameters can be passed through registers or on the stack, or both.
// - Allocating storage for local variables on the stack.
// - Returning a value—the value returned from a subroutine needs to be stored in a place where the caller can find it.Typically, this is a dedicated CPU register.
// - Using registers without interfering with other functions—the registers that a subroutine uses need to be restored to the state they were in before the subroutine was called.
//
// The exact details about how function calls are carried out are specified by something called calling conventions.
// Calling conventions differ among CPU architectures and compilers and
// are one of the major parts that constitutes an application binary interface (ABI).

// When a function is being called, a call frame (or activation frame) for that function is being created.
// - The parameters passed to the function.
// - The local variables of the function.
// - A snapshot of the registers that we intend to useand therefore need to restore before returning.
// - A return address that links back to the place in memory where the caller invoked the function from.
// - An optional frame pointer that points back to the top of the caller's call frame. Frame pointers are useful for debuggers
//   when inspecting the stack. We will not discuss frame pointers further in this book.
// A call frame stored on the stack is usually called a stack frame.

// Different with subroutines, we need to keep the current value of the variable,
// and also remember where in the coroutine we should continue executing the next time the coroutine is resumed.
// This information is placed into something called a coroutine frame.

// There are two types of coroutines:
// 
// 1. Stackful coroutines:
//    Stackful coroutines have a separate side stack (similar to a thread) that
//    contains the coroutine frame and the nested call frames.
//    Each side stack has its own SP. Each call to a stackful coroutine creates a separate side stack with a unique stack pointer.
// 2. Stackless coroutines:
//    need to store the coroutine frame somewhere else (typically on the heap) and
//    then use the stack of the currently executing thread to store nested call frames.
//    The caller doesn't know that it is calling a coroutine that will suspend and resume. (treat it as subroutines)
//    Therefore, the coroutine itself needs to create the coroutine frame and copy the parameters and registers from the call frame.
//    A stackless coroutine has a separate coroutine frame (usually on the heap) that contains the state necessary for resuming the coroutine.

// Stackful coroutines are sometimes called fibers, and in the programming language Go, they are called goroutines.

// There is also a category of threads called user-level threads or green threads.
// These are lightweight threads that don't involve kernel mode switching
// (because they run in user mode and are therefore unknown to the kernel).
// Fibers are one example of user-level threads.

// Stackless coroutines and state machines are tightly related.
// It's possible to transform a state machine into a coroutine and vice versa.
// However, some areas where state machines are usually applied are parsing,
// gesture recognition, and I/O multiplexing, to mention a few.
// These are all areas where stackless coroutine can really shine both in terms of expressiveness and performance.

// Memory footprint:
// In summary, stackful coroutines demand a big initial memory allocation for the coroutine frame and the side stack,
// or need to support a growing stack. Stackless coroutines only need to allocate memory for the coroutine frame.
// The memory footprint of calling a coroutine can be summarized as follows:
// 
// Stackless: Coroutine frame
// Stackful : Coroutine frame + call stack

// Context switching:
// Suspending and resuming coroutines is also a kind of context switch because we are switching between multiple concurrent flows.
// Switching between coroutines is substantially faster than switching between processes and OS threads,
// partly because it doesn't involve any system calls that require the CPU to run in kernel mode.
// 
// But, in general, a stackful coroutine has a more expensive context switch operation since it has more information to save and
// restore during suspend and resume compared to a stackless coroutine.
// Resuming a stackless coroutine is comparable to a normal function call.

// Stackful coroutines can be seen as non-preemptive user-level threads,
// whereas stackless coroutines offer a way to write state machines in a direct imperative fashion
// using the keywords await and yield to specify the suspend points.

//-------------------------------------------------------------------------

// The coroutines added to C++20 are 'stackless' coroutines.
// There are options to use stackful coroutines in C++ as well by using third-party libraries.
// The most well-known cross-platform library is Boost.Fiber.

auto subroutine()
{
	// Sequence of statements ...
	auto a = 2;
	a += 2;
	std::cout << "Print: " << a << '\n';

	return; // Stop and return control to caller
}

// Pseudo code
auto coroutine()
{
	// auto value = 10;
	// await something;        // Suspend/Resume point
	// // ...
	// yield value++;          // Suspend/Resume point
	// yield value++;          // Suspend/Resume point
	// // ...
	return;
}

void SubroutinesAndCoroutines()
{
	subroutine();
	coroutine();
}