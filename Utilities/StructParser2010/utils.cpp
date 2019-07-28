#include "stdio.h"
#include "tokenizer.h"


FILE *fopen_nofail(const char *pFileName, const char *pModes)
{
	FILE *pRetVal = fopen(pFileName, pModes);

	Tokenizer::StaticAssertf(pRetVal != NULL, "Couldn't open file %s(%s)", pFileName, pModes);

	return pRetVal;
}
