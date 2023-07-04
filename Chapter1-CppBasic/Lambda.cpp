#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>

// lambda expressions generate function objects.
// A function object is an instance of a class that has the call operator, operator()(), defined.

// To understand what a lambda expression consists of, you can view it as a regular class with restrictions:

// The class only consists of one member function
// The capture clause is a combination of the class' member variables and its constructor

void download_webpage( 
	const char* url,
	void ( *callback )( int, const char* ) // can pass lambda as function pointer here!
)
{
	callback( 2, "No result!" );
}

void Lambda()
{
	auto v = std::vector{ 1, 3, 2, 5, 4 };

	auto num_3 = std::count( v.begin(), v.end(), 3 );

	// This is so called stateless lambda, since it doesn't reference any external state.
	// They don't copy or reference any variables from outside the lambda and therefore don't need any internal state.
	auto num_above_3 = std::count_if( v.begin(), v.end(), +[] ( int i )
	{
		return i > 3;
	} );

	std::cout << num_3 << "\n";
	std::cout << num_above_3 << "\n";
	
	auto add_to = 2;
	// Use temporary variable in lambda
	// &ref = add_to makes a temporary reference 'ref' to 'add_to'
	auto temp_use = [ numbers = std::vector<int>{1, 2, 3}, &ref = add_to ]() // C++17
	{
		for ( auto i : numbers )
			std::cout << i + ref << "\n";
	};

	temp_use();
	std::cout << "\n";

	auto counter_func = [ counter = 1 ] () mutable
	{
		std::cout << counter++ << "\n";
	};
	counter_func(); // Output: 1
	counter_func(); // Output: 2
	counter_func(); // Output: 3

	// + force lambda to have no capture
	// This way lambda is converted into regular function pointer.
	auto lambda_with_no_capture = +[] ( int result, const char* str )
	{
		// Process result and str
		std::cout << result << ": " << str << "\n";
	};

	download_webpage( "test", lambda_with_no_capture );
	
	// Why lambda with capture can _NOT_ be pass as regular function pointer?
	// Because lambda with capture will be instantiated to different types, but a function pointer can only point to one type at a time.
	// Only lambda without capture can be instantiated to just one types.

	// Since C++20, lambdas without captures are default-constructible and assignable.
	// By using decltype, it's now easy to construct different lambda objects that have the same type.
	// However, this only applies to lambdas without captures.
	// Lambdas with captures have their own unique type.
	// Even if two lambda functions with captures are plain clones of each other,
	// they still have their own unique type.Therefore, it's not possible to assign one lambda with captures to another lambda.

	auto x = [] {};          // A lambda without captures
	auto y = x;              // Assignable
	decltype( y ) z = {};    // Default-constructible
	static_assert( std::is_same_v<decltype( x ), decltype( y )> ); // passes
	static_assert( std::is_same_v<decltype( x ), decltype( z )> ); // passes

	// What is important here is that what is captured by the lambda does not affect its signature,
	// and therefore both lambdas withand without captures can be assigned to the same std::function variable.
	auto with_capture = [ &ref = add_to ] ( int a )
	{
		std::cout << "A = " << a + ref << "\n";
	};

	auto without_capture = +[] ( int a )
	{
		std::cout << "A = " << a << "\n";
	};

	// std::function can wrap both stateful and stateless lambda which share same signature!
	auto all_accept = std::function<void( int )>{ with_capture };
	all_accept(2);
	all_accept = without_capture; // reassign
	all_accept(2);

	// A std::function has a few performance losses compared to a function object constructed by a lambda expression directly.
	// 1. Prevented inline optimizations.
	// 2. Dynamically allocated memory for captured variables.
	// 3. Additional run-time computation.

	// Using typename (C++20)
	auto template_func = []<typename Val>( Val v )
	{
		return v + 1;
	};

	template_func( 1 );
	template_func( 1.25 );
}