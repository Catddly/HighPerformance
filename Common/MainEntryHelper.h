#pragma once

#include <iostream>

#define Entry( FUNC_NAME ) std::cout<< #FUNC_NAME << " Begin \n\n"; \
	FUNC_NAME(); \
	std::cout << "\n" << #FUNC_NAME << " End \n\n"