#include <iostream>
#include <vector>

void g( double )
{
    std::cout << "g(double)\n";
}

// Note: Non-dependent names are looked up and bound at the point of template definition.
// Binding of dependent names is postponed until lookup takes place.
//
// As discussed in lookup, the lookup of a dependent name used in a template is postponed until the template arguments are known.

template<class T>
struct S
{
    void f() const
    {
        // "g" is a non-dependent name, because it doesn't come from template parameter T, bound now.
        // even though g(int) is the best function overload match.
        // Implicit type conversion happened ( int -> double )
        g( 1 );
    }
};

void g( int )
{
    std::cout << "g(int)\n";
}

//template <class T>
//void S<T>::f() const
//{
//    // template definition is now here!
//    // it will bound g(int) for the best match!
//    g( 1 );
//}

template <typename T>
struct B
{
    int     i;
};

// https://www.modernescpp.com/index.php/dependent-types

// Dependent names are looked up at the point of the template instantiation.
template<typename T>
struct X : B<T>                 // "B<T>" is dependent on T
{
    typename T::A* pa;          // "T::A" is dependent on T
    void f( B<T>* pb )
    {
        static int i = B<T>::i; // "B<T>::i" is dependent on T
        pb->j++;                // "pb->j" is dependent on T
    }
};

// Now, the fun starts. A dependent name can be a type, a non-type, or a template parameter.

template <typename T>
class A
{
    A* p1;      // A is the current instantiation
    A<T>* p2;   // A<T> is the current instantiation
    ::A<T>* p4; // ::A<T> is the current instantiation
    A<T*> p3;   // A<T*> is _NOT_ the current instantiation, because it use A<T*>
};
template <typename T>
class A<T*>
{
    A<T*>* p1;  // A<T*> is the current instantiation
    A<T>* p2;   // A<T> is _NOT_ the current instantiation, because it use A<T>
};

template <int I>
struct B1
{
    static const int my_I = I;
    static const int my_I2 = I + 0;
    static const int my_I3 = my_I;
    B1<my_I>* b3;  // B<my_I> is the current instantiation
    B1<my_I2>* b4; // B<my_I2> is not the current instantiation, because my_I2 is not directly refer to I
    B1<my_I3>* b5; // B<my_I3> is the current instantiation
};

// If a dependent name could be a type, a non-type, or a template, you have to give the compiler a hint.

template <typename T>
void test()
{
    //std::vector<T>::const_iterator* p1 = nullptr;        // (1) because compiler will treat const_iterator as a non-type as default, * will not mean a pointer but a multiplication.
                                                           //     this makes compiler confused and produce error.
    typename std::vector<T>::const_iterator* p2 = nullptr; // (2) use typename to tell compiler const_iterator is a type.
}

template <typename T>
struct S1
{
    template <typename U>
    void func()
    {}
};

template <typename T>
void func2()
{
    S1<T> s;
    //s.func<T>();           // (1) because compiler will treat func as a non-type as default.
                             //     this makes compiler confused.
    s.template func<T>();    // (2) use template to tell compiler this dependent name is a template.
}

void DependentTemplateArgument()
{
    g( 1 );  // calls g(int)

    S<int> s;
    s.f(); // calls g(double)
}