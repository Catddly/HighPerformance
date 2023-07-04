#pragma once

#include <string>

template <typename T>
std::string TypeName()
{
	#if defined(_MSC_VER)
	std::string fullName( __FUNCSIG__ );
	size_t leftAngleBracket = fullName.find_last_of( '<' );
	size_t rightAngleBracket = fullName.find_last_of( '>' );
	std::string name = fullName.substr( leftAngleBracket + 1, rightAngleBracket - leftAngleBracket - 1 );
	if ( name.find( "struct" ) != std::string::npos )
		return name.substr( 7, name.size() - 7 );
	else if ( name.find( "class" ) != std::string::npos )
		return name.substr( 6, name.size() - 6 );
	else
		return name;
	#endif
}

#define TYPE_CATEGORY( ty ) TypeName<decltype(ty)>()
#define VALUE_CATEGORY( ty ) TypeName<decltype((ty))>()