#include "autoenums.h"
#include "metadictionary.h"
#include "stashtable.h"



char *AutoEnumStringFromIndex(AUTO_ENUM_INFO *pAutoEnumInfo, intptr_t iIndex)
{
	if (!pAutoEnumInfo)
	{
		return NULL;
	}

	if (!pAutoEnumInfo->pInitValuePairs)
	{
		if (iIndex < 0 || iIndex >= pAutoEnumInfo->iNumEntries)
		{
			return NULL;
		}

		return pAutoEnumInfo->ppEnumEntries[iIndex];
	}
	else
	{
		int iCurBestPair = -1;
		int iCurBestValue = -2000000000;

		int iRealIndex, iNextRealIndex;

		int i;

		for (i=0; pAutoEnumInfo->pInitValuePairs[i].iIndex != -1; i++)
		{
			int iCurValue = pAutoEnumInfo->pInitValuePairs[i].iValue;

			if (iCurValue > iCurBestValue && iCurValue <= iIndex)
			{
				iCurBestValue = iCurValue;
				iCurBestPair = i;
			}
		}

		if (iCurBestPair == -1)
			{
			if (iIndex < 0 || iIndex >= pAutoEnumInfo->iNumEntries)
			{
				return NULL;
			}

			return pAutoEnumInfo->ppEnumEntries[iIndex];
		}

		iRealIndex = iIndex - iCurBestValue + pAutoEnumInfo->pInitValuePairs[iCurBestPair].iIndex;

		iNextRealIndex = pAutoEnumInfo->pInitValuePairs[iCurBestPair + 1].iIndex;

		if (iNextRealIndex != -1 && iRealIndex >= iNextRealIndex)
		{
			return NULL;
		}


		if (iRealIndex < 0 || iRealIndex >= pAutoEnumInfo->iNumEntries)
		{
			return NULL;
		}

		return pAutoEnumInfo->ppEnumEntries[iRealIndex];
	}
}

bool AutoEnumIndexFromString(intptr_t *pOutVal, AUTO_ENUM_INFO *pAutoEnumInfo, const char *pString)
{
	intptr_t iIndex;


	if (!pAutoEnumInfo)
	{
		return false;
	}

	
	if (!stashFindInt((StashTable)(pAutoEnumInfo->pStashTable), pString, &iIndex))
	{
		return false;
	}
		


	if (!pAutoEnumInfo->pInitValuePairs)
	{
		*pOutVal = iIndex;
		return true;
	}
	else
	{
		if (pAutoEnumInfo->pInitValuePairs[0].iIndex > iIndex)
		{
			*pOutVal = iIndex;
			return true;
		}
		else
		{
			int iPairToUse = 0;

			while (pAutoEnumInfo->pInitValuePairs[iPairToUse + 1].iIndex <= iIndex && pAutoEnumInfo->pInitValuePairs[iPairToUse + 1].iIndex != -1)
			{
				iPairToUse++;
			}					

			*pOutVal =  iIndex - pAutoEnumInfo->pInitValuePairs[iPairToUse].iIndex + pAutoEnumInfo->pInitValuePairs[iPairToUse].iValue;
			return true;
		}
	}
	
	return false;
}


bool AutoEnumDictionaryS2P(void **ppOutPointer, const char *pString, const void *pDictionaryData)
{
	return AutoEnumIndexFromString((intptr_t*)ppOutPointer, (AutoEnumHandle)pDictionaryData, pString);
}

bool AutoEnumDictionaryP2S(char outString[METADICTIONARY_MAX_STRING_LENGTH], void *pInPointer, const void *pDictionaryData)
{
	char *pString = AutoEnumStringFromIndex((AutoEnumHandle)pDictionaryData, (int)((U64)pInPointer));

	if (pString)
	{
		strncpy(outString, pString, METADICTIONARY_MAX_STRING_LENGTH);
		return true;
	}

	return false;
}




/*

void SimpleEnumTest(int iMin, int iMax, char *pName)
{
	int i;

	for (i=iMin; i <= iMax; i++)
	{
		char tempString[METADICTIONARY_MAX_STRING_LENGTH];
		int iOutIndex;

		MetaDictionaryPointerToString(tempString, (void*)((U64)i), pName);

		iOutIndex = (int)(U64)MetaDictionaryStringToPointer(tempString, pName);

		printf("In %s, converted %d to %s and back to %d\n", pName, i, tempString, iOutIndex);


	}
}*/

void AutoEnums_Init(AUTO_ENUM_INFO *pAutoEnums, int iNumAutoEnums)
{
	int i;

	for (i=0; i < iNumAutoEnums; i++)
	{
		int j;
	
		(StashTable)(pAutoEnums[i].pStashTable) = stashTableCreateWithStringKeys(8, StashDeepCopyKeys);

	
		for (j=0; j < pAutoEnums[i].iNumEntries; j++)
		{
			char *pEntry = pAutoEnums[i].ppEnumEntries[j];

			do
			{
				char *pNextEntry = strchr(pEntry, '|');

				if (pNextEntry)
				{
					*pNextEntry = 0;
					pNextEntry++;
				}

				stashAddInt((StashTable)(pAutoEnums[i].pStashTable), pEntry, j, true);

				pEntry = pNextEntry;
			} while (pEntry);
		}

		MetaDictionaryAddDictionary(pAutoEnums[i].pEnumName, (void*)(&pAutoEnums[i]), AutoEnumDictionaryS2P, AutoEnumDictionaryP2S);
	}

}

