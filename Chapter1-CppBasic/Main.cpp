#include "MainEntryHelper.h"

// Manually Header
void AutoTypeDeduction();
void CopyEpsilon();
void ConstCorrectness();
void RuleOfFive();
void RValueMemberFunction();
void PassbyValueWhenApplicable();
void ErrorHandlering();
void Lambda();

int main()
{
	Entry( AutoTypeDeduction );
	Entry( CopyEpsilon );
	Entry( ConstCorrectness );
	Entry( RuleOfFive );
	Entry( RValueMemberFunction );
	Entry( PassbyValueWhenApplicable );
	Entry( ErrorHandlering );
	Entry( Lambda );
}