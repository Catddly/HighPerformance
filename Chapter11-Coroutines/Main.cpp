#include "MainEntryHelper.h"

// Note: We say in our code that we will wait,
// continue some other flow, and then come back when ready.
// Coroutines allow us to do that.

// Manually Header
void SubroutinesAndCoroutines();
void UseCoroutines();
void Generator();

int main()
{
	Entry( SubroutinesAndCoroutines );
	Entry( UseCoroutines );
	Entry( Generator );
}