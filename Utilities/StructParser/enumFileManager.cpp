#include "enumFileManager.h"
#include "identifierdictionary.h"
#include "sourceparser.h"
#include "strutils.h"

#define OLDNAME_STRING "OLDNAME"

EnumFileManager::EnumFileManager()
{
	m_iNumEnums = 0;
	m_EnumFileName[0] = 0;
	m_bSomethingChanged = false;
	m_bCurrentlyAddingAnEnum = false;
}

EnumFileManager::~EnumFileManager()
{
	int i;

	for (i=0; i < m_iNumEnums; i++)
	{
		DeleteSingleEnum(&m_Enums[i]);
	}
}

typedef enum
{
	RW_PARSABLE = RW_COUNT,
};

static char *sEnumManagerReservedWords[] =
{
	"PARSABLE",
	NULL
};
void EnumFileManager::SetProjectPathAndName(char *pProjectPath, char *pProjectName)
{
	sprintf(m_EnumFileName, "%s\\%s_enums_autogen.c", pProjectPath, pProjectName);
	sprintf(m_ShortEnumFileName, "%s_enums_autogen", pProjectName);
}



bool EnumFileManager::LoadStoredData(bool bForceReset)
{
	assert(!m_bCurrentlyAddingAnEnum);

	if (bForceReset)
	{
		m_bSomethingChanged = true;
		return false;
	}

	Tokenizer tokenizer;

	if (!tokenizer.LoadFromFile(m_EnumFileName))
	{
		m_bSomethingChanged = true;
		return false;
	}

	if (!tokenizer.IsStringAtVeryEndOfBuffer("#endif"))
	{
		m_bSomethingChanged = true;
		return false;
	}


	tokenizer.SetExtraReservedWords(sEnumManagerReservedWords);

	Token token;
	enumTokenType eType;



	do
	{
		eType = tokenizer.GetNextToken(&token);
		
		assert(eType != TOKEN_NONE);
	}
	while (!(eType == TOKEN_RESERVEDWORD && token.iVal == RW_PARSABLE));

	tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_INT, 0, "Didn't find total number of enums");

	m_iNumEnums = token.iVal;


	int iWhichEnum;

	for (iWhichEnum = 0; iWhichEnum < m_iNumEnums; iWhichEnum++)
	{
		SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iWhichEnum];

		tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find enum name");
		tokenizer.Assert(strlen(token.sVal) < ENUM_MAX_NAME_LENGTH, "enum name too long");

		strcpy(pCurEnum->enumName, token.sVal);

		tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find source file name");
		tokenizer.Assert(strlen(token.sVal) < MAX_PATH, "source file name too long");

		strcpy(pCurEnum->sourceFileName, token.sVal);

		tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_INT, 0, "Didn't find number of enum members");
		pCurEnum->iNumEntries = token.iVal;

		tokenizer.Assert(pCurEnum->iNumEntries >= 0, "invalid number found for num entries");

		if (pCurEnum->iNumEntries == 0)
		{
			pCurEnum->pEntries = NULL;
		}
		else
		{
			pCurEnum->pEntries = new ENUM_ENTRY_STRUCT[pCurEnum->iNumEntries];
			assert(pCurEnum->pEntries);

			int i;

			for (i=0;i < pCurEnum->iNumEntries; i++)
			{
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, ENUM_MAX_NAME_LENGTH - 1, "Didn't find enum struct name");

				strcpy(pCurEnum->pEntries[i].codeName, token.sVal);
		
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_INT, 0, "Didn't find number of extra enum names");
				pCurEnum->pEntries[i].iNumOtherNames = token.iVal;

				int j;

				for (j=0; j < pCurEnum->pEntries[i].iNumOtherNames; j++)
				{
					tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, ENUM_MAX_NAME_LENGTH - 1, "Didn't find extra enum name");
					strcpy(pCurEnum->pEntries[i].otherNames[j], token.sVal);
				}
			}
		}
	}

	return true;
}

void EnumFileManager::DeleteSingleEnum(SINGLE_ENUM_STRUCT *pEnum)
{
	assert(!m_bCurrentlyAddingAnEnum);

	int i;

	if (pEnum->iNumEntries)
	{	
		delete pEnum->pEntries;
	}
}

void EnumFileManager::ResetSourceFile(char *pSourceFileName)
{
	assert(!m_bCurrentlyAddingAnEnum);

	int i = 0;

	while (i < m_iNumEnums)
	{
		if (AreFilenamesEqual(pSourceFileName, m_Enums[i].sourceFileName))
		{
			m_bSomethingChanged = true;

			DeleteSingleEnum(&m_Enums[i]);

			m_iNumEnums--;

			if (i < m_iNumEnums)
			{
				memcpy(&m_Enums[i], &m_Enums[m_iNumEnums], sizeof(SINGLE_ENUM_STRUCT));
			}
		}
		else
		{
			i++;
		}
	}
}



void EnumFileManager::CreateNewEnum(char *pSourceFileName, char *pEnumName, int iNumEntries, Tokenizer *pTokenizer)
{
	assert(!m_bCurrentlyAddingAnEnum);

	assert(m_iNumEnums < MAX_ENUMS);

	SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[m_iNumEnums++];

	strcpy(pCurEnum->sourceFileName, pSourceFileName);
	strcpy(pCurEnum->enumName, pEnumName);
	pCurEnum->iNumEntries = iNumEntries;

	m_iNumEntriesAdded = 0;

	if (iNumEntries == 0)
	{
		pCurEnum->pEntries = NULL;
	}
	else
	{
		pCurEnum->pEntries = new ENUM_ENTRY_STRUCT[iNumEntries];
	}

	m_bSomethingChanged = true;
	m_bCurrentlyAddingAnEnum = true;

	pCurEnum->prefix[0] = 0;

	strcpy(pCurEnum->fileName, pTokenizer->GetCurFileName());
	pCurEnum->iLineNum = pTokenizer->GetCurLineNum();

}

void EnumFileManager::AddEntry(char *pEntryName, bool bHasSpecifiedInitValue, char *pInitValueIdentifier, int iInitValue, Tokenizer *pTokenizer)
{
	pTokenizer->Assert(m_iNumEnums > 0, "Enum appears to be empty");
	pTokenizer->Assert(m_iNumEntriesAdded < m_Enums[m_iNumEnums - 1].iNumEntries, "Enum appears to be badly formatted");
	pTokenizer->Assert(m_bCurrentlyAddingAnEnum, "Enum parsing out of sync");

	int iLen = (int)strlen(pEntryName);

	m_Enums[m_iNumEnums - 1].ppEntries[m_iNumEntriesAdded] = new char [iLen + 1];

	strcpy(m_Enums[m_iNumEnums - 1].ppEntries[m_iNumEntriesAdded], pEntryName);

	if (bHasSpecifiedInitValue)
	{
		pTokenizer->Assert(m_iNumTempInitValuePairs < MAX_INIT_VALUE_PAIRS, "Too many specified values for enum");

		m_TempInitValuePairs[m_iNumTempInitValuePairs].iIndex = m_iNumEntriesAdded;

		if (pInitValueIdentifier)
		{
			pTokenizer->Assert(strlen(pInitValueIdentifier) < ENUM_MAX_NAME_LENGTH, "enum init identifier too long");
			strcpy(m_TempInitValuePairs[m_iNumTempInitValuePairs].identifier, pInitValueIdentifier);
		}
		else
		{
			m_TempInitValuePairs[m_iNumTempInitValuePairs].iValue = iInitValue;
			m_TempInitValuePairs[m_iNumTempInitValuePairs].identifier[0] = 0;
		}

		m_iNumTempInitValuePairs++;
	}

	m_iNumEntriesAdded++;

	if (m_iNumEntriesAdded == m_Enums[m_iNumEnums - 1].iNumEntries)
	{
		if (m_iNumTempInitValuePairs)
		{
			m_Enums[m_iNumEnums - 1].pInitValuePairs = new INIT_VALUE_PAIR[m_iNumTempInitValuePairs + 1];

			m_Enums[m_iNumEnums - 1].pInitValuePairs[m_iNumTempInitValuePairs].iIndex = -1;

			memcpy(m_Enums[m_iNumEnums - 1].pInitValuePairs, m_TempInitValuePairs, sizeof(INIT_VALUE_PAIR) * m_iNumTempInitValuePairs);
		}

		RemoveCommonPrefix(&m_Enums[m_iNumEnums - 1]);

		m_bCurrentlyAddingAnEnum = false;
	}
}

void EnumFileManager::EnumAssert(SINGLE_ENUM_STRUCT *pEnum, bool bCondition, char *pErrorMessage)
{
	if (!bCondition)
	{
		printf("%s(%d) : error S0000 : (StructParser) %s\n", pEnum->fileName, pEnum->iLineNum, pErrorMessage);
		fflush(stdout);
		Sleep(100);
		exit(1);
	}
}





void EnumFileManager::WriteOutData(IdentifierDictionary *pDictionary)
{
	assert(!m_bCurrentlyAddingAnEnum);

	if (!m_bSomethingChanged)
	{
		return;
	}
	
	FixUpInitIdentifiers();


	FILE *pOutFile = fopen(m_EnumFileName, "wb");

	assert(pOutFile);

	fprintf(pOutFile, "//This file should contain all enum name data... autogenerated by structParser\n\n//DO NOT MODIFY OR CHECK IN THIS FILE. EVER. FOR ANY REASON. donot""checkin \n#include \"AutoEnums.h\"\n");

	int iEnumNum;

	for (iEnumNum=0; iEnumNum < m_iNumEnums; iEnumNum++)
	{
		SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iEnumNum];

		int i;

		for (i=0; i < pCurEnum->iNumEntries; i++)
		{
			fprintf(pOutFile, "\n static char sAutoEnums_%s_%d[] = \"%s\";\n", pCurEnum->enumName, i, pCurEnum->ppEntries[i]);
		}

		fprintf(pOutFile, "\nstatic char *sAutoEnumNames_%s[] = // enum defined in %s\n{\n", pCurEnum->enumName, pCurEnum->sourceFileName);
		for (i=0; i < pCurEnum->iNumEntries; i++)
		{
			fprintf(pOutFile, "\tsAutoEnums_%s_%d,\n", pCurEnum->enumName, i);
		}

		fprintf(pOutFile, "};\n\n");

		if (pCurEnum->pInitValuePairs)
		{
			fprintf(pOutFile, "\n\nstatic AUTO_ENUM_INIT_VALUE_PAIR sAutoEnumInitValuePairs_%s[] =\n{\n", pCurEnum->enumName);

			i = -1;

			do
			{	
				i++;
				fprintf(pOutFile, "\t{\n\t\t%d, %d\n\t},\n", pCurEnum->pInitValuePairs[i].iIndex, pCurEnum->pInitValuePairs[i].iIndex == -1 ? 0 : pCurEnum->pInitValuePairs[i].iValue);
			}
			while (pCurEnum->pInitValuePairs[i].iIndex != -1);

			fprintf(pOutFile, "};\n\n");
		}
	}

	fprintf(pOutFile, "int gNumAutoEnums%s = %d;\n\n", m_pParent->GetShortProjectName(), m_iNumEnums);

	fprintf(pOutFile, "AUTO_ENUM_INFO gAllAutoEnums%s[] =\n{\n", m_pParent->GetShortProjectName());

	if (m_iNumEnums)
	{
		for (iEnumNum=0; iEnumNum < m_iNumEnums; iEnumNum++)
		{
			SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iEnumNum];

			fprintf(pOutFile, "\t{\n\t\t\"%s\",\n\t\tsAutoEnumNames_%s,\n",
				pCurEnum->enumName, pCurEnum->enumName);
			fprintf(pOutFile, "\t\t%d,\n", pCurEnum->iNumEntries);

			if (pCurEnum->pInitValuePairs)
			{
				fprintf(pOutFile, "\t\tsAutoEnumInitValuePairs_%s,\n", pCurEnum->enumName);
			}
			else
			{
				fprintf(pOutFile, "\t\tNULL,\n");
			}

			fprintf(pOutFile, "\t\tNULL,\n");

			fprintf(pOutFile, "\t},\n");
		}
	}
	else
	{
		fprintf(pOutFile, "\t0\n");
	}

	fprintf(pOutFile, "};\n");


	fprintf(pOutFile, "\n\n#if 0\n%s\n", sEnumManagerReservedWords[0]);

	fprintf(pOutFile, "%d //num enums\n\n\n", m_iNumEnums);

	for (iEnumNum=0; iEnumNum < m_iNumEnums; iEnumNum++)
	{
		SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iEnumNum];

		fprintf(pOutFile, "\"%s\" // enum name\n", pCurEnum->enumName);
		fprintf(pOutFile, "\"%s\" // source file name\n", pCurEnum->sourceFileName);

		if (pCurEnum->pInitValuePairs == NULL)
		{
			fprintf(pOutFile, "0 // num init value pairs\n");
		}
		else
		{
			int iNumPairs = 0;

			while (pCurEnum->pInitValuePairs[iNumPairs].iIndex != -1)
			{
				iNumPairs++;
			}

			fprintf(pOutFile, "%d // num init value pairs\n", iNumPairs);

			int i;

			for (i=0; i < iNumPairs; i++)
			{
				fprintf(pOutFile, "%d %d // the %dth entry has value %d\n", 
					pCurEnum->pInitValuePairs[i].iIndex, 
					pCurEnum->pInitValuePairs[i].iValue, 
					pCurEnum->pInitValuePairs[i].iIndex, 
					pCurEnum->pInitValuePairs[i].iValue);
			}
		}

		fprintf(pOutFile, "%d // num enum entries\n", pCurEnum->iNumEntries);

		int i;

		for (i=0; i < pCurEnum->iNumEntries; i++)
		{
			fprintf(pOutFile, "\"%s\" // entry %d\n",
				pCurEnum->ppEntries[i], i);
		}

		fprintf(pOutFile, "\n\n");
	}

	fprintf (pOutFile, "#endif\n");



	fclose(pOutFile);

	m_pParent->CompileCFile(m_ShortEnumFileName);
}

void EnumFileManager::FoundMagicWord(char *pSourceFileName, Tokenizer *pTokenizer, IdentifierDictionary *pDictionary)
{
	
	char enumName[MAX_PATH];
	int iNumEntries = 0;

	enumName[0] = 0;

	Token token;
	enumTokenType eType;

	eType = pTokenizer->GetNextToken(&token);

	if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_TYPEDEF)
	{
		eType = pTokenizer->GetNextToken(&token);
	}

	pTokenizer->Assert(eType == TOKEN_IDENTIFIER && strcmp(token.sVal, "enum") == 0, "expected [typedef] enum after AUTO_ENUM");


	eType = pTokenizer->CheckNextToken(&token);
	
	if (eType == TOKEN_IDENTIFIER)
	{
		pTokenizer->Assert(strlen(token.sVal) < ENUM_MAX_NAME_LENGTH, "enum name too long");
		strcpy(enumName, token.sVal);
		pTokenizer->GetNextToken(&token);
	}
		
	pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_LEFTBRACE, "Expected { after AUTO_ENUM");

	pTokenizer->SaveLocation();

	do
	{
		eType = pTokenizer->GetNextToken(&token);

		if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_RIGHTBRACE)
		{
			break;
		}

		pTokenizer->Assert(eType == TOKEN_IDENTIFIER, "Expected identifier in enum list");

		iNumEntries++;

		//walk forward until we get to a comma or }. Eat the comma, leave the } alone
		do
		{
			eType = pTokenizer->CheckNextToken(&token);

			if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_RIGHTBRACE)
			{
				break;
			}

			eType = pTokenizer->GetNextToken(&token);

			if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_COMMA)
			{
				break;
			}
		} while (1);

	} while (1);

	eType = pTokenizer->CheckNextToken(&token);

	if (eType == TOKEN_IDENTIFIER)
	{
		if (enumName[0])
		{
			pTokenizer->Assert(strcmp(token.sVal, enumName) == 0, "The same enum has two different names");
		}
		else
		{
			strcpy(enumName, token.sVal);
		}
	}

	pTokenizer->Assert(enumName[0] != 0, "AUTO_ENUM appears to have no name");


	CreateNewEnum(pSourceFileName, enumName, iNumEntries, pTokenizer);

	pDictionary->AddIdentifier(enumName, pSourceFileName, IDENTIFIER_ENUM);

	pTokenizer->RestoreLocation();


	do
	{
		Token mainEntryToken;

		char initValueString[ENUM_MAX_NAME_LENGTH];
		char *pInitValueString = NULL;

		bool bFoundValue = false;
		int iValue = 0;

		eType = pTokenizer->GetNextToken(&mainEntryToken);

		if (eType == TOKEN_RESERVEDWORD && mainEntryToken.iVal == RW_RIGHTBRACE)
		{
			break;
		}

		pTokenizer->Assert(eType == TOKEN_IDENTIFIER, "Expected identifier in enum list");
		
		eType = pTokenizer->CheckNextToken(&token);

		if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_EQUALS)
		{
			pTokenizer->GetNextToken(&token);

			eType = pTokenizer->CheckNextToken(&token);

			if (eType == TOKEN_IDENTIFIER)
			{
				bFoundValue = true;

				pTokenizer->GetNextToken(&token);

				pTokenizer->Assert(strlen(token.sVal) < ENUM_MAX_NAME_LENGTH, "enum init identifier too long");

				strcpy(initValueString, token.sVal);
				pInitValueString = initValueString;


			}
			else
			{

				pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_INT, 0, "Expected integer or identifier after =");

				bFoundValue = true;

				iValue = token.iVal;
			}
		}

		eType = pTokenizer->CheckNextToken(&token);

		pTokenizer->Assert(eType == TOKEN_RESERVEDWORD && (token.iVal == RW_RIGHTBRACE || token.iVal == RW_COMMA), "expected , or } after enum entry");

		if (token.iVal == RW_COMMA)
		{
			pTokenizer->GetNextToken(&token);
		
			GetOldNames(pTokenizer, mainEntryToken.sVal);

		}

		AddEntry(mainEntryToken.sVal, bFoundValue, pInitValueString, iValue, pTokenizer);

	} while (1);
}

void EnumFileManager::RemoveCommonPrefix(SINGLE_ENUM_STRUCT *pEnum)
{
	bool bFoundAtLeastTwoStrings = false;
	char workString[TOKENIZER_MAX_STRING_LENGTH] = "";

	int i;

	for (i=0; i < pEnum->iNumEntries; i++)
	{
		char *pCurString = pEnum->ppEntries[i];

		while (pCurString)
		{
			if (workString[0])
			{
				bFoundAtLeastTwoStrings = true;

				int iOffset = 0;

				while (workString[iOffset] && workString[iOffset] == pCurString[iOffset])
				{
					iOffset++;
				}

				workString[iOffset] = 0;
			}
			else
			{
				strcpy(workString, pCurString);
			}

			pCurString = strchr(pCurString, '|');

			if (pCurString)
			{
				pCurString++;
			}
		}
	}

	if (!bFoundAtLeastTwoStrings)
	{
		return;
	}

	int iPrefixLength = (int)strlen(workString);

	if (!iPrefixLength)
	{
		return;
	}

	strcpy(pEnum->prefix, workString);

	for (i=0; i < pEnum->iNumEntries; i++)
	{
		char *pCurString  = pEnum->ppEntries[i];

		while (pCurString)
		{
			memmove(pCurString, pCurString + iPrefixLength, strlen(pCurString) - iPrefixLength + 1);
			
			pCurString = strchr(pCurString, '|');

			if (pCurString)
			{
				pCurString++;
			}
		}
	}	
}

void EnumFileManager::GetOldNames(Tokenizer *pTokenizer, char *pMainString)
{
	int iCurLen = (int)strlen(pMainString);
	char workString[TOKENIZER_MAX_STRING_LENGTH];
	strcpy(workString, pMainString);

	Token token;
	enumTokenType eType;

	do
	{
		eType = pTokenizer->CheckNextToken(&token);

		if (eType == TOKEN_IDENTIFIER && strcmp(token.sVal, OLDNAME_STRING) == 0)
		{
			pTokenizer->GetNextToken(&token);
			pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_LEFTPARENS, "Expected ( after OLDNAME");
			pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_IDENTIFIER, 0, "Expected identifier after OLDNAME(");

			int iNewLen = (int)strlen(token.sVal);

			pTokenizer->Assert(iNewLen + iCurLen + 1 < TOKENIZER_MAX_STRING_LENGTH, "Combined length of OLDNAMEs too long");

			sprintf(workString + iCurLen, "|%s", token.sVal);
			iCurLen += iNewLen + 1;

			pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_RIGHTPARENS, "Expected ) after OLDNAME(x");

		}
		else
		{

			strcpy(pMainString, workString);
			return;
		}
	}
	while (1);
}





/*
this function searches for all enums that have init values like 
{
	foo,
	bar = SOME_IDENTIFIER,
}
and attempts to resolve the identifier as an enum from some other enum
*/

void EnumFileManager::FixUpInitIdentifiers()
{
	int iEnumNum;

	for (iEnumNum=0; iEnumNum < m_iNumEnums; iEnumNum++)
	{
		SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iEnumNum];

		FixUpInitIdentifiersSingleEnum(pCurEnum);
	}
}

void EnumFileManager::FixUpInitIdentifiersSingleEnum(SINGLE_ENUM_STRUCT *pCurEnum)
{
	int i;

	EnumAssert(pCurEnum, !pCurEnum->bCurrentlyDoingIdentifierFixup, "Enums appaear to recursively define each other");

	pCurEnum->bCurrentlyDoingIdentifierFixup = true;


	for (i=0; i < pCurEnum->iNumEntries; i++)
	{
		if (pCurEnum->pInitValuePairs)
		{
			INIT_VALUE_PAIR *pPair = pCurEnum->pInitValuePairs;

			while (pPair->iIndex != -1)
			{
				if (pPair->identifier[0])
				{
					int iVal;
					bool bFound = TryToGetEnumValue(pPair->identifier, &iVal);

					if (bFound)
					{
						pPair->iValue = iVal;
						pPair->identifier[0] = 0;
					}
					else
					{
						char errorString[1024];

						sprintf(errorString, "Couldn't decode identifier %s. Only other AUTO_ENUM values are allowed, or literal ints\n", pPair->identifier);
						EnumAssert(pCurEnum, 0, errorString);
					}
				}

				pPair++;
			}
		}
	}

	pCurEnum->bCurrentlyDoingIdentifierFixup = false;
}

bool EnumFileManager::TryToGetEnumValue(char *pString, int *iVal)
{
	int iEnumNum;

	for (iEnumNum=0; iEnumNum < m_iNumEnums; iEnumNum++)
	{
		SINGLE_ENUM_STRUCT *pCurEnum = &m_Enums[iEnumNum];

		int iEntryNum;

		for (iEntryNum=0; iEntryNum < pCurEnum->iNumEntries; iEntryNum++)
		{
			char testString[ENUM_MAX_NAME_LENGTH];

			sprintf(testString, "%s%s", pCurEnum->prefix, pCurEnum->ppEntries[iEntryNum]);

			if (strcmp(testString, pString) == 0)
			{
				if (pCurEnum->pInitValuePairs)
				{
					INIT_VALUE_PAIR *pPair = pCurEnum->pInitValuePairs;

					if (pPair->iIndex > iEntryNum)
					{
						*iVal = iEntryNum;
						return true;
					}

					while (pPair->iIndex != -1 && pPair->iIndex <= iEntryNum)
					{
						pPair++;
					}

					pPair--;

					if (pPair->identifier[0])
					{
						FixUpInitIdentifiersSingleEnum(pCurEnum);

						EnumAssert(pCurEnum, pPair->identifier[0] == 0, "Couldn't resolve enum init identifier");
					}

					*iVal = pPair->iValue + iEntryNum - pPair->iIndex;

					return true;

				}
				else
				{
					*iVal = iEntryNum;
					return true;
				}
			}
		}
	}


	return false;
}
	









