#include <iostream>
// #include <experimental/propagate_const>

class FooCompile
{
public:
    FooCompile( int* ptr ) : ptr_{ ptr }
    {}

    auto set_ptr_val( int v ) const
    {
        *ptr_ = v; // Compiles despite function being declared const!
    }

    auto print() const
    {
        std::cout << *ptr_ << "\n";
    }
private:

    // only the pointer is const, but the object it points to is _NOT_ const.
    int* ptr_{};
};

/// Note that propagate_const only applies to pointers, and pointer-like classes such as std::shared_ptr and std::unique_ptr, but not std::function.

//class FooNotCompile
//{
//public:
//    FooNotCompile( int* ptr ) : ptr_{ ptr }
//    {}
//    auto set_ptr( int* p ) const
//    {
//        ptr_ = p;  // Will not compile, as expected
//    }
//    auto set_val( int v ) const
//    {
//        val_ = v;  // Will not compile, as expected
//    }
//    auto set_ptr_val( int v ) const
//    {
//        *ptr_ = v; // Will not compile, const is propagated
//    }
//private:
//    std::experimental::propagate_const<int*> ptr_ = nullptr;
//    int val_{};
//};

void ConstCorrectness()
{
    auto i = 0;
    const auto foo = FooCompile{ &i };
    foo.set_ptr_val( 42 );
    foo.print();
}