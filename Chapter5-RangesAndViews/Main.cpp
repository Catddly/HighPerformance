#include "MainEntryHelper.h"

// Manually Header
void LazyEvalViews();
void ViewsBefore();

int main()
{
	Entry( LazyEvalViews );
	Entry( ViewsBefore );
}