#include "MainEntryHelper.h"

// Manually Header
void MemoryUsage();
void MemoryOwnership();
void SmallObjectOptimization();
void CustomAllocator();

// Note: memory in programmers' eyes
//
// The physical memory of a computer is shared among all the processes running on a system.
// If one process uses a lot of memory, the other processes will most likely be affected.
// But from a programmer's perspective, we usually don't have to bother about the memory that is being used by other processes.
// This isolation of memory is due to the fact that most operating systems today are virtual memory operating systems,
// which provide the illusion that a process has all the memory for itself.
// Each process has its own virtual address space.
//
// MMU (memory management unit): Used every time we access memory by address. mapping virtual address to physical memory address.
// Page Fault: Happen when OS fails map memory memory into disk memory.
// Paging: Every time when a page is dirty(modified), it needs to be written to disk before it can be replaced by another page.
// Thrashing: Happen when a system runs low on physical memory and is, therefore, constantly paging.

// Not all operating systems that support virtual memory support paging.
// iOS, for example, does have virtual memory but dirty pages are never stored on disk;
// only clean pages can be evicted from memory. If the main memory is full,
// iOS will start terminating processes until there is enough free memory again.
// Android uses a similar strategy. One reason for not writing memory pages
// back to the flash storage of the mobile devices is that it drains the battery,
// and it also shortens the lifespan of the flash storage itself.

int main()
{
	Entry( MemoryUsage );
	Entry( MemoryOwnership );
	Entry( SmallObjectOptimization );
	Entry( CustomAllocator );
}