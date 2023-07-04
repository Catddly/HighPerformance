#include <string>
#include <iostream>

// A fundamental aspect of error handling, 
// regardless of programming language, 
// is to distinguish between programming errors (also known as bugs) and runtime errors. 
// Runtime errors can be further divided into recoverable runtime errors and unrecoverable runtime errors. 
// An example of an unrecoverable runtime error is stack overflow (see Chapter 7, Memory Management).
// When an unrecoverable error occurs, the program typically terminates immediately,
// so there is no point in signaling these types of errors.
// However, some errors might be considered recoverable in one type of application but unrecoverable in others.

// Design By Contract !!!
// A contract is a set of rules between the caller of some function and the function itself (the callee).

// - Procondition
// - Postcondition
// - invariant

struct Widget
{
    Widget()
    {
        // Initialize object…
        // Check class invariant
    }
    ~Widget()
    {
        // Check class invariant
        // Destroy object…
    }
    auto some_func()
    {
        // Check precondition (including class invariant)
        // Do the actual work…
        // Check postcondition (including class invariant)
    }
};

// Rule:
// 1. Always reset moved object to the default constructed state.

// Runtime-Error: Recoverable Error Example:

// This can _NOT_ be recovered
//void func( std::string& str )
//{
//    str += f1();  // Could throw
//    str += f2();  // Could throw
//}

std::string f1()
{
    return "Hello!";
}

std::string f2()
{
    return " No!";
}

std::string f2_throw()
{
    throw std::bad_alloc{};
    return "";
}

// Use copy-and-swap to recover
void func( std::string& str )
{
    auto tmp = std::string{ str };  // Copy
    tmp += f1();                    // Mutate copy, may throw
    tmp += f2();                    // Mutate copy, may throw
    std::swap( tmp, str );          // Swap, never throws
}

void func_throw( std::string& str )
{
    auto tmp = std::string{ str };  // Copy
    tmp += f1();                    // Mutate copy, may throw
    tmp += f2_throw();              // Mutate copy, may throw
    std::swap( tmp, str );          // Swap, never throws
}

void ErrorHandlering()
{
    auto str = std::string{ "" };

    try
    {
        func( str );
        func_throw( str ); // str won't be modified
    }
    catch ( ... )
    {
        std::cerr << "Some Error Happened!\n";
    }

    // str is perfect!
    std::cout << str << "\n";
}