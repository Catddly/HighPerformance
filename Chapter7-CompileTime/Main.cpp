#include "MainEntryHelper.h"

// Note: In a more general sense, metaprogramming is a technique where we write code that transforms or generates some other code. 
// In its simplest and most common form,
// template metaprogramming in C++ is used to generate functions, values, and classes that accept different types.
// A template is said to be instantiated when the compiler uses that template to generate a class or a function.
// Constant expressions are evaluated by the compiler to generate constant values.

// Two compiler parse phases:
// 1. An initial phase, where templates and constant expressions produce regular C++ code of functions, classes, and constant values.
//    This phase is usually called constant evaluation.
// 2. A second phase, where the compiler eventually compiles the regular C++ code into machine code.

// Manually Header
void TypeTemplate();
void TypeTraits();
void ConstraintAndConcept();
void DependentTemplateArgument();
void Cpp20NTTP();

int main()
{
	Entry( TypeTemplate );
	Entry( TypeTraits );
	Entry( ConstraintAndConcept );
	Entry( DependentTemplateArgument );
	Entry( Cpp20NTTP );
}