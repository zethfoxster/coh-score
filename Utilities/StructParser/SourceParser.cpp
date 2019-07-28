#include "sourceparser.h"
#include "strutils.h"

#include "MagicCommandManager.h"
#include "structparser.h"
#include "AutoTransactionManager.h"
#include "autorunmanager.h"
#include "windows.h"
#include "direct.h"
#include "autoTestManager.h"
#include "latelinkmanager.h"


#define MAX_PROJECTS_ONE_SOLUTION 256

#define MAX_WILDCARD_MAGIC_WORDS 16

#define STRUCTPARSERSTUB_PROJ "StructParserStub"

char sTime[] = __TIME__;

typedef enum
{
	RW_FILE = RW_COUNT,
	RW_RELATIVEPATH,
	RW_CONFIGURATION,
	RW_ADDITIONALINCLUDEDIRECTORIES,
	RW_PREPROCESSORDEFINITIONS,
	RW_OUTPUTDIRECTORY,
	RW_OBJECTFILE,
	RW_NAME,
	RW_TOOL,
	RW_PROPERTYSHEETS,
	RW_INTERMEDIATEDIRECTORY,
};

static char *sProjectReservedWords[] =
{
	"File",
	"RelativePath",
	"Configuration",
	"AdditionalIncludeDirectories",
	"PreprocessorDefinitions",
	"OutputDirectory",
	"ObjectFile",
	"Name",
	"Tool",
	"InheritedPropertySheets",
	"IntermediateDirectory",

	NULL
};




typedef enum
{
	RW_GLOBAL = RW_COUNT,
	RW_PROJECT,
	RW_PROJECTDEPENDENCIES,
	RW_ENDPROJECTSECTION,
	RW_ENDPROJECT,
};

static char *sSolutionReservedWords[] =
{
	"Global",
	"Project",
	"ProjectDependencies",
	"EndProjectSection",
	"EndProject",
	NULL
};

//must be all caps
static char *sFileNamesToExclude[] =
{
	"STDTYPES.H",
	NULL
};

static char *sProjectNamesToExclude[] =
{
	"GimmeDLL",
	NULL
};

bool ShouldFileBeExcluded(char *pFileName)
{
	char tempFileName[MAX_PATH];
	strcpy(tempFileName, pFileName);

	if (strstr(pFileName, "Program Files"))
	{
		return true;
	}

	char *pTemp;
	char *pSimpleFileName = pTemp = tempFileName;

	while (*pTemp)
	{
		if ((*pTemp == '/' || *pTemp == '\\') && *(pTemp + 1))
		{
			pSimpleFileName = pTemp + 1;
		}

		pTemp++;
	}

	MakeStringUpcase(pSimpleFileName);

	int i = 0;

	while (sFileNamesToExclude[i])
	{
		if (AreFilenamesEqual(pSimpleFileName, sFileNamesToExclude[i]))
		{
			return true;
		}


		i++;
	}

	if (strstr(pSimpleFileName, "AUTOGEN"))
	{
		return true;
	}

	return false;
}




SourceParser::SourceParser()
{
	m_iNumSourceParsers = 6;

	m_pSourceParsers[0] = NULL;
	m_pSourceParsers[1] = NULL;
	m_pSourceParsers[2] = NULL;
	m_pSourceParsers[3] = NULL;
	m_pSourceParsers[4] = NULL;
	m_pSourceParsers[5] = m_pAutoRunManager = NULL;

	m_iNumProjectFiles = 0;

	memset(m_iNumDependencies, 0, sizeof(m_iNumDependencies));

	m_pFileListLoader = new FileListLoader;
	m_pFileListWriter = new FileListWriter;

	memset(m_bFilesNeedToBeUpdated, 0, sizeof(m_bFilesNeedToBeUpdated));

	memset(m_iExtraDataPerFile, 0, sizeof(m_iExtraDataPerFile));

	m_iNumDependentLibraries = 0;

	m_FoundAutoGenFile1 = false;
	m_FoundAutoGenFile1 = false;
	m_bIsAnExecutable = false;

	m_bProjectFileChanged = false;
	m_bCleanBuildHappened = false;

	m_pFirstVar = NULL;

}

void SourceParser::CreateParsers(void)
{
	m_pSourceParsers[0] = new MagicCommandManager;
	m_pSourceParsers[1] = new StructParser;
	m_pSourceParsers[2] = new AutoTransactionManager;
	m_pSourceParsers[3] = new AutoTestManager;
	m_pSourceParsers[4] = new LateLinkManager;

	//AutoRunManager should generally be last
	m_pSourceParsers[5] = m_pAutoRunManager = new AutoRunManager;
}


SourceParser::~SourceParser()
{
	int i;

	for (i=0; i < m_iNumSourceParsers; i++)
	{
		if (m_pSourceParsers[i])
		{
			delete m_pSourceParsers[i];
		}
	}

	while (m_pFirstVar)
	{
		SourceParserVar *pNext = m_pFirstVar->pNext;
		delete m_pFirstVar->pVarName;
		delete m_pFirstVar->pValue;
		delete m_pFirstVar;
		m_pFirstVar = pNext;
	}

	delete m_pFileListLoader;
	delete m_pFileListWriter;
}

bool SourceParser::IsLibraryXBoxExcluded(char *pLibName)
{
	if (strstr(pLibName, "GLRenderLib"))
	{
		return true;
	}

	return false;
}


void SourceParser::ProcessSolutionFile(bool bRecursivelyCallStructParser)
{
	Tokenizer tokenizer;
	
	int iNumProjects = 0;
	char *pProjectNames[MAX_PROJECTS_ONE_SOLUTION];
	char *pProjectIDStrings[MAX_PROJECTS_ONE_SOLUTION];
	char *pProjectFullPaths[MAX_PROJECTS_ONE_SOLUTION];

	bool bResult = tokenizer.LoadFromFile(m_SolutionPath);
		
	bool bFoundOurProject = false;
	bool bFoundStubProj = false;
	bool bDependsOnStubProj = false;

	Tokenizer::StaticAssert(bResult, "Couldn't load solution file");

	//set reservedwords used for parsing through .vcproj file
	tokenizer.SetExtraReservedWords(sSolutionReservedWords);

	Token token;
	enumTokenType eType;

	while ((eType = tokenizer.GetNextToken(&token)) != TOKEN_NONE)
	{
		if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_GLOBAL)
		{
			break;
		}

		if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_PROJECT)
		{
			tokenizer.Assert(iNumProjects < MAX_PROJECTS_ONE_SOLUTION, "Too many projects in .sln file");

			do
			{
				eType = tokenizer.GetNextToken(&token);
				tokenizer.Assert(eType != TOKEN_NONE, "Unexpected end of .sln file while parsing project");
			}
			while (!(eType == TOKEN_RESERVEDWORD && token.iVal == RW_EQUALS));

			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, MAX_PATH, "Expected project name");

			if (StringIsInList(token.sVal, sProjectNamesToExclude) && strcmp(token.sVal, m_ShortenedProjectFileName) != 0)
			{
				//skip this project
			}
			else
			{

				pProjectNames[iNumProjects] = new char[token.iVal + 1];
				strcpy(pProjectNames[iNumProjects], token.sVal);

				if (strcmp(token.sVal, STRUCTPARSERSTUB_PROJ) == 0)
				{
					bFoundStubProj = true;
				}

				if (strcmp(token.sVal, m_ShortenedProjectFileName) == 0)
				{
					bFoundOurProject = true;
					tokenizer.SaveLocation();
				}


				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_COMMA, "Expected , after project name");
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, MAX_PATH, "Expected project full path");

				pProjectFullPaths[iNumProjects] = new char[token.iVal + 1];
				strcpy(pProjectFullPaths[iNumProjects], token.sVal);

				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_COMMA, "Expected , after project full path");
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, MAX_PATH, "Expected project ID string");

				pProjectIDStrings[iNumProjects] = new char[token.iVal + 1];
				strcpy(pProjectIDStrings[iNumProjects], token.sVal);

				iNumProjects++;
			}
		}
	}

	tokenizer.Assert(bFoundOurProject, "Didn't find current project referenced in .sln file");

	tokenizer.RestoreLocation();

	do
	{
		eType = tokenizer.GetNextToken(&token);

		tokenizer.Assert(eType != TOKEN_NONE, "unexpected end of file before EndProject");

		if (token.eType == TOKEN_RESERVEDWORD && token.iVal == RW_ENDPROJECT)
		{
			break;
		}

		if (token.eType == TOKEN_RESERVEDWORD && token.iVal == RW_PROJECTDEPENDENCIES)
		{
			do
			{
				eType = tokenizer.GetNextToken(&token);
				tokenizer.Assert(eType != TOKEN_NONE, "unexpected end of file before EndProjectSection");

				if (token.eType == TOKEN_RESERVEDWORD && token.iVal == RW_ENDPROJECTSECTION)
				{
					break;
				}

				if (token.eType == TOKEN_RESERVEDWORD && token.iVal == RW_LEFTBRACE)
				{
					char tempString[1024] = "{";
					
					tokenizer.SetDontParseInts(true);

					do
					{
						eType = tokenizer.GetNextToken(&token);

						tokenizer.Assert(eType != TOKEN_NONE, "unexpected end of file in project UID");

						if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_RIGHTBRACE)
						{
							break;
						}

						tokenizer.Assert(eType == TOKEN_IDENTIFIER || eType == TOKEN_RESERVEDWORD && token.iVal == RW_MINUS, 
							"found unexpected characters while parsing projectUID");
						tokenizer.StringifyToken(&token);

						strcat(tempString, token.sVal);
					} while (1);

					tokenizer.SetDontParseInts(false);

					strcat(tempString, "}");

					int i;
					bool bFound =false;

					for (i=0; i < iNumProjects; i++)
					{
						if (strcmp(tempString, pProjectIDStrings[i]) == 0)
						{
							if (strcmp(pProjectNames[i], STRUCTPARSERSTUB_PROJ) != 0)
							{
								tokenizer.Assert(m_iNumDependentLibraries < MAX_DEPENDENT_LIBRARIES, "too many dependent libraries");
								strcpy(m_DependentLibraryNames[m_iNumDependentLibraries], pProjectNames[i]);
								strcpy(m_DependentLibraryFullPaths[m_iNumDependentLibraries], pProjectFullPaths[i]);
								m_bExcludeLibrariesFromXBOX[m_iNumDependentLibraries] = IsLibraryXBoxExcluded(pProjectNames[i]);
								
								
								m_iNumDependentLibraries++;
							}
							else
							{
								bDependsOnStubProj = true;
							}
							break;
						}
					}

					do
					{
						eType = tokenizer.GetNextToken(&token);

						tokenizer.Assert(eType != TOKEN_NONE, "unexpected end of file in project UID");

					} while (!(eType == TOKEN_RESERVEDWORD && token.iVal == RW_RIGHTBRACE));

				}
			} while (1);

		}
	} while (1);

	if (bRecursivelyCallStructParser)
	{
		int i;

		char solutionDirectory[MAX_PATH];
		strcpy(solutionDirectory, m_SolutionPath);
		int iSolutionLength = (int)strlen(solutionDirectory);
		while (solutionDirectory[iSolutionLength - 1] != '\\' && solutionDirectory[iSolutionLength - 1] != '/')
		{
			solutionDirectory[iSolutionLength - 1] = 0;
			iSolutionLength--;
		}

		
		for (i=0; i < iNumProjects; i++)
		{
			if (_stricmp(pProjectNames[i], m_ShortenedProjectFileName) != 0)
			{
				char tempString1[1024];
				char tempString2[1024];
				sprintf(tempString1, "%s.%s|%s.Build.0 = ", 
					pProjectIDStrings[i], m_pCurConfiguration, m_pCurTarget);
				sprintf(tempString2, "%s.%s|%s.ActiveCfg = ", 
					pProjectIDStrings[i], m_pCurConfiguration, m_pCurTarget);

				if ((tokenizer.SetReadHeadAfterString(tempString1) || tokenizer.SetReadHeadAfterString(tempString2)) &&
					!(IsLibraryXBoxExcluded(pProjectNames[i]) && strstr(m_pCurTarget, "Xbox")) )
				{
					char configToUse[128];
					char platformToUse[128];

					char fullPath[MAX_PATH];

					char *pReadHead = tokenizer.GetReadHead();
					char *pFirstBar = strchr(pReadHead, '|');


					tokenizer.Assert(pFirstBar != NULL, "Couldn't find | while looking for config|platform");
					
					tokenizer.Assert(pFirstBar - pReadHead < 128, "Overflow while looking for config|platform");
					
					strncpy(configToUse, pReadHead, pFirstBar - pReadHead);
					configToUse[pFirstBar - pReadHead] = 0;

					char *pFirstNewLine = strchr(pFirstBar, '\n');

					tokenizer.Assert(pFirstNewLine != NULL, "Couldn't find \\n while looking for config|platform");
					tokenizer.Assert(pFirstNewLine - pFirstBar < 128, "Overflow while looking for config|platform");

					strncpy(platformToUse, pFirstBar + 1, pFirstNewLine - pFirstBar - 1);
					platformToUse[pFirstNewLine - pFirstBar - 2] = 0;

					assembleFilePath(fullPath, solutionDirectory, pProjectFullPaths[i]);

					int iFullPathLen = (int)strlen(fullPath);
					while (fullPath[iFullPathLen - 1] != '\\' && fullPath[iFullPathLen - 1] != '/')
					{
						fullPath[iFullPathLen - 1] = 0;
						iFullPathLen--;
					}

					char projectName[256];
					sprintf(projectName, "%s.vcproj", pProjectNames[i]);

					SourceParser *pRecurseParser = new SourceParser;

					_chdir(fullPath);

					printf("About to recursively call: %s X %s X %s X %s X %s X %s X %s\n",
						m_Executable, fullPath, projectName, platformToUse, configToUse, m_pCurVCDir, m_SolutionPath);

					pRecurseParser->ParseSource(fullPath, projectName, platformToUse, configToUse, m_pCurVCDir, m_SolutionPath, m_Executable);

					delete pRecurseParser;
				}
			}
		}

		ProcessProjectFile();
	}
	else
	{
		if (bFoundStubProj && !bDependsOnStubProj)
		{
			char errorString[1024];
			sprintf(errorString, "StructParserStub project exists, but project %s doesn't depend on it", m_ShortenedProjectFileName);
			Tokenizer::StaticAssert(0, errorString);
		}
	}



	int i;
	
	for (i=0; i < iNumProjects; i++)
	{
		delete [] pProjectNames[i];
		delete [] pProjectIDStrings[i];
		delete [] pProjectFullPaths[i];
	}
}

void SourceParser::CheckForRequiredFiles(char *pFileName)
{
	char *pShortName = GetFileNameWithoutDirectories(pFileName);

	while (pShortName[0] == '\\' || pShortName[0] == '/')
	{
		pShortName++;
	}

	if (_stricmp(m_AutoGenFile1Name, pShortName) == 0)
	{
		m_FoundAutoGenFile1 = true;
	}
	else if (_stricmp(m_AutoGenFile2Name, pShortName) == 0)
	{
		m_FoundAutoGenFile2 = true;
	}
}

void SourceParser::GetAdditionalStuffFromPropertySheets(char *pDirsAlreadyFound, char *pPropertySheetNames, char *pToolName, int iReservedWordToFind)
{
	char *pTemp;

	if ((pTemp = strstr(pDirsAlreadyFound, "$(NOINHERIT)")))
	{
		*pTemp = 0;
		return;
	}

	while (pPropertySheetNames && pPropertySheetNames[0])
	{
		char fileName[MAX_PATH];
		strcpy(fileName, pPropertySheetNames);

		if ((pTemp = strchr(fileName, ';')))
		{
			*pTemp = 0;
		}

		if ((pTemp = strchr(pPropertySheetNames, ';')))
		{
			pPropertySheetNames = pTemp + 1;
		}
		else
		{
			pPropertySheetNames = NULL;
		}

		if (strstr(fileName, "UpgradeFromVC71.vsprops"))
		{
			continue;
		}

		Tokenizer tokenizer;
		tokenizer.LoadFromFile(fileName);
		tokenizer.SetExtraReservedWords(sProjectReservedWords);
		
		Token token;
		enumTokenType eType;
	
		bool bDone = false;

		do
		{
			eType = tokenizer.GetNextToken(&token);

			if (eType == TOKEN_NONE)
			{
				break;
			}

			if (pToolName	? eType == TOKEN_STRING && strcmp(token.sVal, pToolName) == 0
							: eType == TOKEN_IDENTIFIER && strcmp(token.sVal, "VisualStudioPropertySheet") == 0 )
			{
				do
				{
					eType = tokenizer.GetNextToken(&token);
					if (eType == TOKEN_NONE)
					{
						bDone = true;
						break;
					}

					if (eType == TOKEN_RESERVEDWORD && token.iVal == iReservedWordToFind)
					{
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Expected = ");
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Expected string");

						if(pDirsAlreadyFound[0])
							strcat(pDirsAlreadyFound, ";");
						strcat(pDirsAlreadyFound, token.sVal);
						bDone = true;
						break;
					}

					if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_GT)
					{
						bDone = true;
						break;
					}
				} while (true);
			}

		} while (!bDone);

	}
}




#define MAX_PROJECTS_ONE_SOLUTION 256

void SourceParser::ProcessProjectFile()
{
	char fullConfigName[100];
	char propertySheetNames[512] = "";


	Tokenizer tokenizer;

	bool bResult = tokenizer.LoadFromFile(m_FullProjectFileName);
		
	Tokenizer::StaticAssert(bResult, "Couldn't open project file");

	//set reservedwords used for parsing through .vcproj file
	tokenizer.SetExtraReservedWords(sProjectReservedWords);

	Token token;
	enumTokenType eType;


	bool bFoundConfiguration = false;
	bool bFoundIntermediateDirectory = false;

	sprintf(fullConfigName, "%s|%s", m_pCurConfiguration, m_pCurTarget);
	m_AdditionalIncludeDirs[0] = 0;
	m_PreprocessorDefines[0] = 0;
	m_ObjectFileDir[0] = 0;

	char compilerToolName[256] = "";

	do
	{
		eType = tokenizer.GetNextToken(&token);

		if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_CONFIGURATION)
		{
			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_NAME, "Didn't find name after configuration");
			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after name");
			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find string after =");

			if (strcmp(token.sVal, fullConfigName) == 0)
			{
				tokenizer.Assert(!bFoundConfiguration, "Found configuration more than once");

				bFoundConfiguration = true;

				char toolName[300] = "";

				do
				{
					eType = tokenizer.GetNextToken(&token);

					tokenizer.Assert(eType != TOKEN_NONE, "EOF found in middle of configuration");


					if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_PROPERTYSHEETS)
					{
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after propertysheets");
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 512, "Didn't find string after =");
						strcpy(propertySheetNames, token.sVal);

						char objectFileDirs[TOKENIZER_MAX_STRING_LENGTH] = "";

						if(bFoundIntermediateDirectory)
							continue;

						GetAdditionalStuffFromPropertySheets(objectFileDirs, propertySheetNames, NULL, RW_INTERMEDIATEDIRECTORY);
						if(objectFileDirs[0])
							bFoundIntermediateDirectory = true;
						else if(!m_ObjectFileDir[0])
							GetAdditionalStuffFromPropertySheets(objectFileDirs, propertySheetNames, NULL, RW_OUTPUTDIRECTORY);

						if(objectFileDirs[0])
						{
							static char *sTempMacros[][2] =
							{
								{
									"$(ConfigurationName)",
									m_pCurConfiguration,
								},
								{
									"$(PlatformName)",
									m_pCurTarget,
								},
								{
									NULL,
									NULL
								}
							};

							strcpy(m_ObjectFileDir, objectFileDirs);
							ReplaceMacrosInPlace(m_ObjectFileDir, sTempMacros);
							PutSlashAtEndOfString(m_ObjectFileDir);
						}
					}
					else if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_TOOL)
					{
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_NAME, "Didn't find name after Tool");
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after name");
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find string after =");

						strcpy(toolName, token.sVal);

						if (strstr(toolName, "LinkerTool"))
						{
							Token nextToken;
							enumTokenType eNextType;

							eNextType = tokenizer.CheckNextToken(&nextToken);

							if (!(eNextType == TOKEN_RESERVEDWORD && nextToken.iVal == RW_SLASH))
							{
								m_bIsAnExecutable = true;
							}
						}

						if (strcmp(toolName, "VCCLCompilerTool") == 0 || strcmp(toolName, "VCCLX360CompilerTool") == 0)
						{
							strcpy(compilerToolName, toolName);
						}
					} 
					else if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_ADDITIONALINCLUDEDIRECTORIES)
					{
						if (strcmp(toolName, "VCCLCompilerTool") == 0 || strcmp(toolName, "VCCLX360CompilerTool") == 0)
						{
							if (m_AdditionalIncludeDirs[0] == 0)
							{
								tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after AdditionalIncludeDirectories");
								tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find string after =");
								strcpy(m_AdditionalIncludeDirs, token.sVal);
								GetAdditionalStuffFromPropertySheets(m_AdditionalIncludeDirs, propertySheetNames, toolName, RW_ADDITIONALINCLUDEDIRECTORIES);
							}
						}
					}
					else if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_PREPROCESSORDEFINITIONS)
					{
						if (strcmp(toolName, "VCCLCompilerTool") == 0 || strcmp(toolName, "VCCLX360CompilerTool") == 0)
						{
							if (m_PreprocessorDefines[0] == 0)
							{
								tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after PreprocessorDefinitions");
								tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find string after =");
								strcpy(m_PreprocessorDefines, token.sVal);
								GetAdditionalStuffFromPropertySheets(m_PreprocessorDefines, propertySheetNames, toolName, RW_PREPROCESSORDEFINITIONS);

								//mixed commas and semicolons screw things up... use all semicolons
								ReplaceCharWithChar(m_PreprocessorDefines, ',', ';');
							}
						}
					}
					else if (eType == TOKEN_RESERVEDWORD && (token.iVal == RW_OUTPUTDIRECTORY || token.iVal == RW_OBJECTFILE || token.iVal == RW_INTERMEDIATEDIRECTORY))
					{
						//an "intermediateDirectory" token is higher priority than "outputdirectory" and/or "objectfile", 
						//which are basically obsolete
						if (token.iVal == RW_INTERMEDIATEDIRECTORY)
						{
							bFoundIntermediateDirectory = true;
						}
						else
						{
							if (bFoundIntermediateDirectory)
							{
								continue;
							}
						}
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after outputdirectory or objectfile");
						tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find string after =");
						strcpy(m_ObjectFileDir, token.sVal);

						static char *sTempMacros[][2] =
						{
							{
								"$(ConfigurationName)",
								m_pCurConfiguration,
							},
							{
								"$(PlatformName)",
								m_pCurTarget,
							},
							{
								NULL,
								NULL
							}
						};

						ReplaceMacrosInPlace(m_ObjectFileDir, sTempMacros);

						PutSlashAtEndOfString(m_ObjectFileDir);
					}
				}
				while (!(eType == TOKEN_RESERVEDWORD && token.iVal == RW_CONFIGURATION));
			}
			else
			{
				tokenizer.GetTokensUntilReservedWord((enumReservedWordType)RW_CONFIGURATION);
			}
		}
		else if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_FILE)
		{

			eType = tokenizer.GetNextToken(&token);

			if (eType == TOKEN_RESERVEDWORD && token.iVal == RW_RELATIVEPATH)
			{
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Didn't find = after reservedword");
				tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Didn't find filename after =");

				//check for the two autogen files that are supposed to be included
				CheckForRequiredFiles(token.sVal);

				int len = (int)strlen(token.sVal);	

				if ((len >= 3 && token.sVal[len - 2] == '.' && (token.sVal[len - 1] == 'h' || token.sVal[len - 1] == 'c')))
				{

					char sourceFileName[256];
					sprintf(sourceFileName, "%s\\%s", m_ProjectPath, token.sVal);

					Tokenizer::StaticAssert(m_iNumProjectFiles < MAX_FILES_IN_PROJECT, "Too many files in project");
					
					if (!ShouldFileBeExcluded(sourceFileName))
					{
						strcpy(m_ProjectFiles[m_iNumProjectFiles++], sourceFileName);
					}
				}
			}
		}
	} while (eType != TOKEN_NONE);

	if (m_PreprocessorDefines[0] == 0 && compilerToolName)
	{
		GetAdditionalStuffFromPropertySheets(m_PreprocessorDefines, propertySheetNames, compilerToolName, RW_PREPROCESSORDEFINITIONS);
		ReplaceCharWithChar(m_PreprocessorDefines, ',', ';');
	}

	if (m_AdditionalIncludeDirs[0] == 0 && compilerToolName)
	{
		GetAdditionalStuffFromPropertySheets(m_AdditionalIncludeDirs, propertySheetNames, compilerToolName, RW_ADDITIONALINCLUDEDIRECTORIES);
	}


	tokenizer.Assert(bFoundConfiguration, "never found configuration");

}

bool SourceParser::NeedToUpdateFile(char *pFileName, int iExtraData, bool bForceUpdateUnlessFileDoesntExist)
{
	HANDLE hFile;

	hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else 
	{
		if (bForceUpdateUnlessFileDoesntExist)
		{
			CloseHandle(hFile);
			return true;
		}
		else
		{	
			bool bNeedToUpdate = false;

			if (m_pFileListLoader->IsFileInList(pFileName))
			{
				FILETIME mainFileTime;

				GetFileTime(hFile, NULL, NULL, &mainFileTime);

				if (CompareFileTime(&mainFileTime, m_pFileListLoader->GetMasterFileTime()) == 1)
				{
					bNeedToUpdate = true;
				}
				else if (iExtraData)
				{
					int i;

					for (i=0; i < m_iNumSourceParsers; i++)
					{
						if (iExtraData & ( 1 << i))
						{
							if (m_pSourceParsers[i]->DoesFileNeedUpdating(pFileName))
							{
								bNeedToUpdate = true;
							}
						}
					}
				}
			}
			else
			{
				bNeedToUpdate = true;
			}

			CloseHandle(hFile);

			return bNeedToUpdate;
		}
	}
}
void SourceParser::MakeAutoGenDirectory()
{
	char commandString[1024];

	sprintf(commandString, "mkdir \"%s\\AutoGen\" > NUL 2>&1", m_ProjectPath);

	system(commandString);

	sprintf(commandString, "mkdir \"%s\\..\\Common\\AutoGen\" > NUL 2>&1", m_ProjectPath);

	system(commandString);
}


bool SourceParser::DoMasterFilesExist()
{
	char fileName[MAX_PATH];
	FILE *pFile;

	sprintf(fileName, "%s\\AutoGen\\%s", m_ProjectPath, m_AutoGenFile1Name);

	pFile = fopen(fileName, "rt");
	if(!pFile)
	{
		return false;
	}
	fclose(pFile);

	sprintf(fileName, "%s\\AutoGen\\%s", m_ProjectPath, m_AutoGenFile2Name);

	pFile = fopen(fileName, "rt");
	if(!pFile)
	{
		return false;
	}
	fclose(pFile);

	return true;
}



void SourceParser::DestroyLegacyMasterFiles(bool bForceBuildAll)
{
	if ( gbLastFWCloseActuallyWrote )
	{
		NukeCObjFile(m_AutoGenFile1Name);
	}

	if (gbLastFWCloseActuallyWrote )
	{
		NukeCObjFile(m_AutoGenFile2Name);
	}
}

void SourceParser::CreateCleanBuildMarkerFile(void)
{
	char fileName[MAX_PATH];

	char fullDirString[MAX_PATH];
	char systemString[1024];

	sprintf(fullDirString, "%s\\%s", m_ProjectPath, m_ObjectFileDir);

	RemoveSuffixIfThere(fullDirString, "/");
	RemoveSuffixIfThere(fullDirString, "\\");
	
	sprintf(systemString, "md \"%s\"  > NUL 2>&1", fullDirString);

	system(systemString);


	sprintf(fileName, "%s\\THIS_FILE_CHECKS_FOR_CLEAN_BUILDS.obj", 
		fullDirString);


	FILE *pFile = fopen(fileName, "wt");
	if (pFile)
	{
		fprintf(pFile, "This file exists so that structparser will know when a clean build happens");
		fclose(pFile);
	}
}

bool SourceParser::DidCleanBuildJustHappen()
{
	char fileName[MAX_PATH];

	sprintf(fileName, "%s\\%s\\THIS_FILE_CHECKS_FOR_CLEAN_BUILDS.obj", 
		m_ProjectPath, m_pFileListLoader->GetObjFileDirectory());

	HANDLE hFile;

	hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return true;;
	}
	
	CloseHandle(hFile);

	return false;
}

void SourceParser::CleanOutAllAutoGenFiles()
{
	char tempString[256];
	sprintf(tempString, "del /Q \"%s\\AutoGen\\*.*\"", m_ProjectPath);
	system(tempString);
	
	sprintf(tempString, "del /Q \"%s\\wiki\\*.*\"", m_ProjectPath);
	system(tempString);
	
	sprintf(tempString, "del /Q \"%s\\..\\Common\\AutoGen\\%s_*.*\"", m_ProjectPath, m_ShortenedProjectFileName);
	system(tempString);
}

bool SourceParser::IsQuickExitPossible()
{
	HANDLE hFile;
	FILETIME fileTime;

	hFile = CreateFile(m_FullProjectFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
		
	GetFileTime(hFile, NULL, NULL, &fileTime);

	if (CompareFileTime(&fileTime, m_pFileListLoader->GetMasterFileTime()) == 1)
	{
		m_bProjectFileChanged = true;
		return false;
	}

	CloseHandle(hFile);



	hFile = CreateFile(m_SolutionPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
		
	GetFileTime(hFile, NULL, NULL, &fileTime);

	if (CompareFileTime(&fileTime, m_pFileListLoader->GetMasterFileTime()) == 1)
	{
		return false;
	}

	CloseHandle(hFile);


	int i;
	int iNumFiles = m_pFileListLoader->GetNumFiles();

	for (i=0; i < iNumFiles; i++)
	{
		if (!m_pFileListLoader->GetNthFileExists(i))
		{
			return false;
		}

		if (CompareFileTime(m_pFileListLoader->GetNthFileTime(i), m_pFileListLoader->GetMasterFileTime()) == 1)
		{
			return false;
		}
	}


	//project file and solution file and all project files are unchanged, quit
	TRACE("Project file, solution file, and all project files are unchanged... quitting\n");
	return true;
}









void SourceParser::ParseSource(char *pProjectPath, char *pProjectFileName, char *pCurTarget, char *pCurConfiguration, char *pCurVCDir, char *pSolutionPath, char *pExecutable)
{
	// remove trailing '\\' from project path
	RemoveSuffixIfThere(pProjectPath, "\\");
	RemoveSuffixIfThere(pProjectPath, "/");

	sprintf(m_FullProjectFileName, "%s\\%s", pProjectPath, pProjectFileName);
	strcpy(m_ProjectPath, pProjectPath);
	strcpy(m_ShortenedProjectFileName,pProjectFileName);
	strcpy(m_SolutionPath, pSolutionPath);
	strcpy(m_Executable, pExecutable);



	m_pCurTarget = pCurTarget;
	m_pCurConfiguration = pCurConfiguration;
	m_pCurVCDir = pCurVCDir;

	TruncateStringAtLastOccurrence(m_ShortenedProjectFileName, '.');

	//if the project file name passed in has "_donotc heckin" stuck on the end of it, then it's a temporary
	//project file generated by Conor's weird auto-build, and we should ignore the _donot checkin
	RemoveSuffixIfThere(m_ShortenedProjectFileName, "_donot""checkin");


	char listFileName[MAX_PATH];
	char shortListFileName[MAX_PATH];
	bool bForceReadAllFiles = false; 

	sprintf(shortListFileName, "%s_%s", m_ShortenedProjectFileName, m_pCurConfiguration);
	MakeStringAllAlphaNum(shortListFileName);


	sprintf(listFileName, "%s\\%s.SPFileList", pProjectPath, shortListFileName);
	sprintf(m_AutoGenFile1Name, "%s_AutoGen_1.c", m_ShortenedProjectFileName);
	sprintf(m_AutoGenFile2Name, "%s_AutoGen_2.cpp", m_ShortenedProjectFileName);
	sprintf(m_SpecialAutoRunFuncName, "_%s_AutoRun_SPECIALINTERNAL", m_ShortenedProjectFileName);


	TRACE("About to start parsing... project %s config %s\n", m_ShortenedProjectFileName, m_pCurConfiguration);

	if (!m_pFileListLoader->LoadFileList(listFileName))
	{
		TRACE("Couldn't load spfilelist file... doing full rebuild\n");
		bForceReadAllFiles = true;
	}
	else
	{
		if (!bForceReadAllFiles)
		{
			/*
			// Autogen files no longer used
			if (!DoMasterFilesExist())
			{
				TRACE("Master files don't exist... doing full rebuild\n");
				bForceReadAllFiles = true;
			}
			else
			*/

			if (DidCleanBuildJustHappen())
			{
				TRACE("Clean build happened... doing full rebuild\n");
				bForceReadAllFiles = true;
				m_bCleanBuildHappened = true;
			}
			else
			{
				if (IsQuickExitPossible())
				{
					return;
				}

				TRACE("Not doing quick exit... something must have changed\n");
			}
		}
	
	
	}




	MakeAutoGenDirectory();

	bool bNeedToRecurseOnAllOtherProjects = (_stricmp(m_ShortenedProjectFileName, STRUCTPARSERSTUB_PROJ) == 0);

	if (bNeedToRecurseOnAllOtherProjects)
	{
		TRACE("This appears to be the StructParserStub project, so all we'll do is recursively\ncall structparser on other projects\n");
	}

	ProcessSolutionFile(bNeedToRecurseOnAllOtherProjects);

	if (bNeedToRecurseOnAllOtherProjects)
	{
		return;
	}

	CreateParsers();


	ProcessProjectFile();

	FindVariablesFileAndLoadVariables();

	CreateCleanBuildMarkerFile();




	bool bAtLeastOneFileUpdated = false;

	if (bForceReadAllFiles)
	{
		TRACE("Erasing all autogenerated files\n");
		CleanOutAllAutoGenFiles();
	}


	


	int i;
	
	for (i=0; i < m_iNumSourceParsers; i++)
	{
		m_pSourceParsers[i]->SetParent(this, i);
		m_pSourceParsers[i]->SetProjectPathAndName(pProjectPath, m_ShortenedProjectFileName);
	}

	
	if (!m_IdentifierDictionary.SetFileNameAndLoad(pProjectPath, m_ShortenedProjectFileName))
	{
		TRACE("Couldn't load identifier dictionary... forcing read all files\n");
		bForceReadAllFiles = true;
	}


	for (i=0; i < m_iNumSourceParsers; i++)
	{		
		if (!m_pSourceParsers[i]->LoadStoredData(bForceReadAllFiles))
		{
			TRACE("Couldn't load stored data %d, forcing read all files\n", i);
			bForceReadAllFiles = true;
		}
	
	}
	
	if (MakeSpecialAutoRunFunction())
	{
		//make sure AutoRunManager has magic internal autorun
		GetAutoRunManager()->AddAutoRunSpecial(m_SpecialAutoRunFuncName, "_SPECIAL_INTERNAL", true, AUTORUN_ORDER_FIRST);
	}
	else
	{
		GetAutoRunManager()->ResetSourceFile("_SPECIAL_INTERNAL");
	}


	//must be after LoadStoredData
	LoadSavedDependenciesAndRemoveObsoleteFiles();

	for (i = 0 ; i < m_iNumProjectFiles; i++)
	{
		bAtLeastOneFileUpdated |= m_bFilesNeedToBeUpdated[i] |= NeedToUpdateFile(m_ProjectFiles[i], m_iExtraDataPerFile[i], bForceReadAllFiles);
	}

/*	if (!bAtLeastOneFileUpdated && !bForceReadAllFiles)
	{
		return;
	}*/





	if (bForceReadAllFiles)
	{
		ProcessAllFiles_ReadAll();
	}
	else
	{
		ProcessAllFiles();

	}

	bool bMasterFilesChanged = false;
	for (i=0; i < m_iNumSourceParsers; i++)
	{
		bMasterFilesChanged |= m_pSourceParsers[i]->WriteOutData();
	}

	m_IdentifierDictionary.WriteOutFile();

	m_pFileListWriter->OpenFile(listFileName, m_ObjectFileDir);

	for (i = 0 ; i < m_iNumProjectFiles; i++)
	{
		m_pFileListWriter->AddFile(m_ProjectFiles[i], m_iExtraDataPerFile[i], m_iNumDependencies[i], m_iDependencies[i]);
	}

	m_pFileListWriter->CloseFile();


	if ((bAtLeastOneFileUpdated && bMasterFilesChanged) || bForceReadAllFiles)
	{
		DestroyLegacyMasterFiles(bForceReadAllFiles);
	}
}

void SourceParser::ScanSourceFile(char *pSourceFile)
{
	char *sMagicWords[MAX_BASE_SOURCE_PARSERS * MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER + 1]; 

	int iNumWildcardMagicWords = 0;
	int iWildcardMagicWordIndices[MAX_WILDCARD_MAGIC_WORDS];

	TRACE("Parsing %s\n", pSourceFile);


	sMagicWords[m_iNumSourceParsers * MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER] = NULL;

	int i, j;

	for (i=0; i < m_iNumSourceParsers; i++)
	{
		for (j=0; j < MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER; j++)
		{
			sMagicWords[i * MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER + j] = m_pSourceParsers[i]->GetMagicWord(j);

			if (StringContainsWildcards(sMagicWords[i * MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER + j]))
			{
				Tokenizer::StaticAssert(iNumWildcardMagicWords < MAX_WILDCARD_MAGIC_WORDS, "Too many wildcard magic words");
				iWildcardMagicWordIndices[iNumWildcardMagicWords++] = i * MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER + j;
			}
		}
	}

	Tokenizer tokenizer;

	bool bResult = tokenizer.LoadFromFile(pSourceFile);

	if (!bResult)
	{
		char errorString[256];
		sprintf(errorString, "Couldn't find file %s\n", pSourceFile);
		Tokenizer::StaticAssert(0, errorString);
	}

	tokenizer.SetCSourceStyleStrings(true);
	tokenizer.SetExtraReservedWords(sMagicWords);
	tokenizer.SetNoNewlinesInStrings(true);
	tokenizer.SetSkipDefines(true);

	Token token;
	enumTokenType eType;

	for (i=0; i < m_iNumSourceParsers; i++)
	{
		m_pSourceParsers[i]->FoundMagicWord(pSourceFile, &tokenizer, MAGICWORD_BEGINNING_OF_FILE, NULL);
	}
	

	do
	{
		eType = tokenizer.GetNextToken(&token);

		if (eType == TOKEN_RESERVEDWORD && token.iVal >= RW_COUNT)
		{
			int iMagicWordNum = token.iVal - RW_COUNT;
			tokenizer.StringifyToken(&token);
			m_pSourceParsers[iMagicWordNum / MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER]->FoundMagicWord(pSourceFile, &tokenizer, iMagicWordNum % MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER, token.sVal);
		}
		else
		{
			int i;

			for (i=0; i < iNumWildcardMagicWords; i++)
			{
				if (eType == TOKEN_IDENTIFIER && DoesStringMatchWildcard(token.sVal, sMagicWords[iWildcardMagicWordIndices[i]]))
				{
					int iMagicWordNum = iWildcardMagicWordIndices[i];
					m_pSourceParsers[iMagicWordNum / MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER]->FoundMagicWord(pSourceFile, &tokenizer, iMagicWordNum % MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER, token.sVal);
					break;
				}
			}
		}
	} while (eType != TOKEN_NONE);

	for (i=0; i < m_iNumSourceParsers; i++)
	{
		m_pSourceParsers[i]->FoundMagicWord(pSourceFile, &tokenizer, MAGICWORD_END_OF_FILE, NULL);
	}

}

void GetStringWithSeparator(char *pOutString, char **ppInString, char separator)
{

	*pOutString = 0;

	while (**ppInString != 0 && **ppInString != separator)
	{
		*pOutString = **ppInString;
		pOutString++;
		(*ppInString)++;
	}

	if (**ppInString == separator)
	{
		(*ppInString)++;
	}

	*pOutString = 0;
}



void PutThingsIntoCommandLine(char *pCommandLine, char *pInputString, char *pPrefixString, bool bStripTrailingSlashes)
{
	int commandLineLength = (int) strlen(pCommandLine);


	do
	{
		char includeDirString[TOKENIZER_MAX_STRING_LENGTH];
	
		//ignore all leading semicolons
		while (pInputString[0] == ';')
		{
			pInputString++;
		}

		GetStringWithSeparator(includeDirString, &pInputString, ';');

		if (includeDirString[0] == 0)
		{
			break;
		}
	
		if (bStripTrailingSlashes)
		{
			RemoveSuffixIfThere(includeDirString, "\\");
			RemoveSuffixIfThere(includeDirString, "/");
		}

		int tempLen = (int)strlen(includeDirString);

		sprintf(pCommandLine + commandLineLength, "%s \"%s\" " ,pPrefixString, includeDirString);

		commandLineLength = (int)strlen(pCommandLine);
	} while (1);
}




void SourceParser::NukeCObjFile(char *pFileName)
{
	char fileNameWithoutExtension[MAX_PATH];
	sprintf(fileNameWithoutExtension, pFileName);
	TruncateStringAtLastOccurrence(fileNameWithoutExtension, '.');

	char buf[MAX_PATH];
	_snprintf(buf, sizeof(buf), "%s%s.obj", m_ObjectFileDir, fileNameWithoutExtension);
	remove(buf);
}
		

		

void ReplaceMacroInPlace(char *pString, char *pMacroToFind, char *pReplaceString)
{
	int iMacroLength = (int)strlen(pMacroToFind);
	int iStringLength = (int)strlen(pString);
	int iReplaceLength = (int)strlen(pReplaceString);

	if (iStringLength < iMacroLength)
	{
		return;
	}

	int i;

	for (i=0; i <= iStringLength - iMacroLength; i++)
	{
		if (strncmp(pMacroToFind, pString + i, iMacroLength) == 0)
		{
			memmove(pString + i + iReplaceLength, pString + i + iMacroLength, iStringLength - (i + iMacroLength) + 1);
			memcpy(pString + i, pReplaceString, iReplaceLength);

			iStringLength += iReplaceLength - iMacroLength;
			i += iReplaceLength - 1;

		}
	}
}

void ReplaceMacrosInPlace(char *pString, char *pMacros[][2])
{
	int i;

	for (i=0; pMacros[i][0]; i++)
	{
		ReplaceMacroInPlace(pString, pMacros[i][0], pMacros[i][1]);
	}
}



//loads in all the dependencies that are stored in the FileListLoader. If one of the two dependent files doesn't exist, sets the other file
//to update. If both exist, store the dependency. Then all the stored dependencies will be processed
void SourceParser::LoadSavedDependenciesAndRemoveObsoleteFiles(void)
{
	int iNumSavedFiles = m_pFileListLoader->GetNumFiles();
	int iSavedFileNum;

	int iSavedFileNumToIndexArray[MAX_FILES_IN_PROJECT];


	for (iSavedFileNum=0; iSavedFileNum < iNumSavedFiles; iSavedFileNum++)
	{
		char *pSavedFileName = m_pFileListLoader->GetNthFileName(iSavedFileNum);

		iSavedFileNumToIndexArray[iSavedFileNum] = FindProjectFileIndex(pSavedFileName);

		if (iSavedFileNumToIndexArray[iSavedFileNum] == -1)
		{
			//this file no longer exists in the project
			m_IdentifierDictionary.DeleteAllFromFile(pSavedFileName);

			int i;

			for (i=0; i < m_iNumSourceParsers; i++)
			{
				m_pSourceParsers[i]->ResetSourceFile(pSavedFileName);
			}
		}
		else
		{
			m_iExtraDataPerFile[iSavedFileNumToIndexArray[iSavedFileNum]] = m_pFileListLoader->GetExtraData(iSavedFileNum);
		}
	}

	//now the iSavedFileNumToIndexArray is properly seeded

	for (iSavedFileNum=0; iSavedFileNum < iNumSavedFiles; iSavedFileNum++)
	{
		int iNumDependencies = m_pFileListLoader->GetNumDependencies(iSavedFileNum);

		int i;

		for (i=0; i < iNumDependencies; i++)
		{
			int iOtherFileNum = m_pFileListLoader->GetNthDependency(iSavedFileNum, i);

			//only process dependencies once, so only if otherFileNum > fileNum
			if (iOtherFileNum > iSavedFileNum)
			{
				int projFileIndex1 = iSavedFileNumToIndexArray[iSavedFileNum];
				int projFileIndex2 = iSavedFileNumToIndexArray[iOtherFileNum];

				if (projFileIndex1 == -1)
				{
					if (projFileIndex2 != -1)
					{
						m_bFilesNeedToBeUpdated[projFileIndex2] = true;
					}
				}
				else if (projFileIndex2 == -1)
				{
					m_bFilesNeedToBeUpdated[projFileIndex1] = true;
				}
				else
				{
					AddDependency(projFileIndex1, projFileIndex2);
				}
			}
		}
	}
}

//returns true if at least one file was set to udpate that was previously not set to update
//
//find all need-to-update files which have dependencies, and set all the other
//files they are dependent on to be need-to-update, and recurse. 
bool SourceParser::ProcessAllLoadedDependencies()
{
	bool bAtLeastOneSetToTrue = false;
	bool bNeedAnotherPass = true;

	while (bNeedAnotherPass)
	{
		bNeedAnotherPass = false;
		int iFileNum;

		for (iFileNum = 0; iFileNum < m_iNumProjectFiles; iFileNum++)
		{
			if (m_bFilesNeedToBeUpdated[iFileNum])
			{
				int i;

				for (i=0; i < m_iNumDependencies[iFileNum]; i++)
				{
					int iOtherFileNum = m_iDependencies[iFileNum][i];

					if (!m_bFilesNeedToBeUpdated[iOtherFileNum])
					{
						bAtLeastOneSetToTrue = true;
						if (iOtherFileNum < iFileNum)
						{
							bNeedAnotherPass = true;
						}

						m_bFilesNeedToBeUpdated[iOtherFileNum] = true;
					}
				}
			}
		}
	}

	return bAtLeastOneSetToTrue;
}

void SourceParser::ClearAllDependenciesForUpdatingFiles(void)
{
	int iFileNum;

	for (iFileNum = 0; iFileNum < m_iNumProjectFiles; iFileNum++)
	{
		if (m_bFilesNeedToBeUpdated[iFileNum])
		{
			m_iNumDependencies[iFileNum] = 0;
		}
	}
}

void SourceParser::AddDependency(int iFile1, int iFile2)
{
	int i;
	
	Tokenizer::StaticAssert(iFile1 != iFile2, "File can't depend on itself");
	
	bool bFound = false;
	for (i=0; i < m_iNumDependencies[iFile1]; i++)
	{
		if (m_iDependencies[iFile1][i] == iFile2)
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		Tokenizer::StaticAssert(m_iNumDependencies[iFile1] < MAX_DEPENDENCIES_SINGLE_FILE, "Too many dependencies");

		m_iDependencies[iFile1][m_iNumDependencies[iFile1]++] = iFile2;
	}

	bFound = false;
	for (i=0; i < m_iNumDependencies[iFile2]; i++)
	{
		if (m_iDependencies[iFile2][i] == iFile1)
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		Tokenizer::StaticAssert(m_iNumDependencies[iFile2] < MAX_DEPENDENCIES_SINGLE_FILE, "Too many dependencies");

		m_iDependencies[iFile2][m_iNumDependencies[iFile2]++] = iFile1;
	}
}


void SourceParser::ProcessAllFiles_ReadAll()
{
	ClearAllDependenciesForUpdatingFiles();
	
	int iFileNum;

	for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
	{
		m_IdentifierDictionary.DeleteAllFromFile(m_ProjectFiles[iFileNum]);

		int i;

		for (i=0; i < m_iNumSourceParsers; i++)
		{
			m_pSourceParsers[i]->ResetSourceFile(m_ProjectFiles[iFileNum]);
		}

		m_iExtraDataPerFile[iFileNum] = 0;
	}

	for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
	{
		ScanSourceFile(m_ProjectFiles[iFileNum]);
	}

	for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
	{
		int i;

		for (i=0; i < m_iNumSourceParsers; i++)
		{
			char *pDependencies[MAX_DEPENDENCIES_SINGLE_FILE];

			int iNumDepenedencies = m_pSourceParsers[i]->ProcessDataSingleFile(m_ProjectFiles[iFileNum], pDependencies);

			int j;

			for (j=0; j < iNumDepenedencies; j++)
			{
				int iOtherFileNum = FindProjectFileIndex(pDependencies[j]);
				char errorString[1024];
				sprintf(errorString, "Dependency file <<%s>> not found (depended on by %s)", pDependencies[j], m_ProjectFiles[iFileNum]);
				
				Tokenizer::StaticAssert(iOtherFileNum != -1 && iOtherFileNum != iFileNum, errorString);

				AddDependency(iFileNum, iOtherFileNum);
			}
		}
	}
}

void SourceParser::ProcessAllFiles()
{

	ProcessAllLoadedDependencies();

	do
	{
		ClearAllDependenciesForUpdatingFiles();
		int iFileNum;


		for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
		{
			if (m_bFilesNeedToBeUpdated[iFileNum])
			{
				m_IdentifierDictionary.DeleteAllFromFile(m_ProjectFiles[iFileNum]);

				int i;

				for (i=0; i < m_iNumSourceParsers; i++)
				{
					m_pSourceParsers[i]->ResetSourceFile(m_ProjectFiles[iFileNum]);
				}

				m_iExtraDataPerFile[iFileNum] = 0;

			}
		}

		for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
		{
			if (m_bFilesNeedToBeUpdated[iFileNum])
			{
				ScanSourceFile(m_ProjectFiles[iFileNum]);
			}
		}

		for (iFileNum=0; iFileNum < m_iNumProjectFiles; iFileNum++)
		{
			if (m_bFilesNeedToBeUpdated[iFileNum])
			{
				int i;

				for (i=0; i < m_iNumSourceParsers; i++)
				{
					char *pDependencies[MAX_DEPENDENCIES_SINGLE_FILE];

					int iNumDependencies = m_pSourceParsers[i]->ProcessDataSingleFile(m_ProjectFiles[iFileNum], pDependencies);

					int j;

					for (j=0; j < iNumDependencies; j++)
					{
						int iOtherFileNum = FindProjectFileIndex(pDependencies[j]);
						char errorString[1024];
						sprintf(errorString, "Dependency file <<%s>> not found", pDependencies[j]);

						Tokenizer::StaticAssert(iOtherFileNum != -1 && iOtherFileNum != iFileNum, errorString);

						AddDependency(iFileNum, iOtherFileNum);
					}
				}
			}
		}
	}
	while (ProcessAllLoadedDependencies());
}


int SourceParser::FindProjectFileIndex(char *pFileName)
{
	int i;

	for (i=0; i < m_iNumProjectFiles; i++)
	{
		if (AreFilenamesEqual(pFileName, m_ProjectFiles[i]))
		{
			return i;
		}
	}

	return -1;
}


void SourceParser::SetExtraDataFlagForFile(char *pFileName, int iFlag)
{
	int iIndex = FindProjectFileIndex(pFileName);

	Tokenizer::StaticAssert(iIndex != -1, "Trying to set extra data flag for nonexistent file");

	m_iExtraDataPerFile[iIndex] |= iFlag;
}

bool SourceParser::ProjectIsClientOrClientOnlyLib(void)
{
	if (strstr(m_PreprocessorDefines, "GAMECLIENT"))
	{
		return true;
	}

	if (strstr(m_PreprocessorDefines, "CLIENTLIBMAKEWRAPPERS"))
	{
		return true;
	}

	return false;
}

	//returns true if the projet is the game server, or a lib that is linked only into the game server
bool SourceParser::ProjectIsGameServerOrGameServerOnlyLib(void)
{
	if (strstr(m_PreprocessorDefines, "GAMESERVER"))
	{
		return true;
	}

	if (strstr(m_PreprocessorDefines, "SERVERLIBMAKEWRAPPERS"))
	{
		return true;
	}

	return false;
}


int SourceParser::GetSVNVersion(char *pFileName)
{

	char tempFileName[MAX_PATH];
	char systemString[1024];
	GetTempFileName(".", "SPR", 0, tempFileName);

	sprintf(systemString, "svn info \"%s\" > \"%s\"", pFileName, tempFileName);
	system(systemString);

	Tokenizer tokenizer;

	if (!tokenizer.LoadFromFile(tempFileName))
	{
		return 0;
	}

	sprintf(systemString, "del \"%s\"", tempFileName);
	system(systemString);

	Token token;
	enumTokenType eType;

	do
	{
		eType = tokenizer.GetNextToken(&token);

		if (eType == TOKEN_NONE)
		{
			return 0;
		}

		if (eType == TOKEN_IDENTIFIER && strcmp(token.sVal, "Revision") == 0)
		{
			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_COLON, "Expected : after Revision");
			tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_INT, 0, "Expected int after Revision:");

			return token.iVal;
		}
	}
	while (1);

	return 0;	
}

bool SourceParser::MakeSpecialAutoRunFunction(void)
{
	if (StringIsInList(m_ShortenedProjectFileName, sProjectNamesToExclude))
	{
		return false;
	}
/*
	if (_stricmp(m_ShortenedProjectFileName, "UtilitiesLib") == 0)
	{
		return true;
	}

	for (i=0; i < m_iNumDependentLibraries; i++)
	{
		if (_stricmp(m_DependentLibraryNames[i], "UtilitiesLib") == 0)
		{
			return true;
		}
	}
*/
	return true;
}

#define MAX_WIKI_CATEGORIES 256

typedef struct SingleCommandStruct
{
	char *pCommandName;
	char *pCommandDescription;
	struct SingleCommandStruct *pNext;
} SingleCommandStruct;

class MasterWikiCommandCategory
{
public:
	MasterWikiCommandCategory(char *pCategoryName);
	~MasterWikiCommandCategory();

	void LoadCommandsFromFile(char *pFileName);
	void SortCommands(void);

	void WriteCommands(FILE *pOutFile);

	char *GetCategoryName();
	bool IsHidden() { return m_bIsHidden; }
	bool m_bProjectsWhichHaveIt[MAX_WIKI_PROJECTS];


private:
	bool m_bIsHidden;
	char m_CategoryName[256];
	SingleCommandStruct *m_pFirstCommand;


};


MasterWikiCommandCategory::MasterWikiCommandCategory(char *pCategoryName)
{
	strcpy(m_CategoryName, pCategoryName);

	m_bIsHidden = (_stricmp(pCategoryName, "hidden") == 0);

	m_pFirstCommand = NULL;
}

MasterWikiCommandCategory::~MasterWikiCommandCategory()
{
	while (m_pFirstCommand)
	{
		SingleCommandStruct *pNext = m_pFirstCommand->pNext;
		delete m_pFirstCommand->pCommandDescription;
		delete m_pFirstCommand->pCommandName;
		delete m_pFirstCommand;
		m_pFirstCommand = pNext;
	}
}

void MasterWikiCommandCategory::LoadCommandsFromFile(char *pFileName)
{
	Tokenizer tokenizer;

	Token token;
	enumTokenType eType;

	tokenizer.SetIgnoreQuotes(true);

	if (!tokenizer.LoadFromFile(pFileName))
	{
		return;
	}
	
	do
	{
		int iOffsetAtBeginningOfCommand;
		int iLineNum;
		char *pReadHeadAtBeginningOfCommand = tokenizer.GetReadHead();

		iOffsetAtBeginningOfCommand = tokenizer.GetOffset(&iLineNum);

		eType = tokenizer.GetNextToken(&token);

		if (eType == TOKEN_NONE)
		{
			return;
		}

		SingleCommandStruct *pNewCommand = new SingleCommandStruct;



		tokenizer.Assert(eType == TOKEN_IDENTIFIER && strcmp(token.sVal, "h4") == 0, "Expected h4");
		tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_DOT, "Expected . after h4");
		tokenizer.AssertNextTokenTypeAndGet(&token, TOKEN_IDENTIFIER, 0, "Expected command name after h4.");

		pNewCommand->pCommandName = new char[token.iVal + 1];
		strcpy(pNewCommand->pCommandName, token.sVal);

		do
		{
			eType = tokenizer.CheckNextToken(&token);

			if (eType == TOKEN_NONE || eType == TOKEN_IDENTIFIER && strcmp(token.sVal, "h4") == 0)
			{
				break;
			}
			else
			{
				eType = tokenizer.GetNextToken(&token);
			}
		}
		while (1);

		int iOffsetAtEndOfCommand;

		iOffsetAtEndOfCommand = tokenizer.GetOffset(&iLineNum);

		pNewCommand->pCommandDescription = new char[iOffsetAtEndOfCommand - iOffsetAtBeginningOfCommand + 1];
		memcpy(pNewCommand->pCommandDescription, pReadHeadAtBeginningOfCommand, iOffsetAtEndOfCommand - iOffsetAtBeginningOfCommand);
		pNewCommand->pCommandDescription[iOffsetAtEndOfCommand - iOffsetAtBeginningOfCommand] = 0;

		pNewCommand->pNext = m_pFirstCommand;
		m_pFirstCommand = pNewCommand;

		NormalizeNewlinesInString(pNewCommand->pCommandDescription);
	} while (1);
}

void MergeSortCommands(SingleCommandStruct **ppList, int iListLen)
{
	int iListLen1;
	int iListLen2;
	SingleCommandStruct *pList1;
	SingleCommandStruct *pList2;
	SingleCommandStruct *pNext;

	int i;

	SingleCommandStruct *pMasterListHead;
	SingleCommandStruct *pMasterListTail;


	if (iListLen < 2)
	{
		return;
	}

	iListLen1 = iListLen / 2;
	iListLen2 = iListLen - iListLen1;

	pList2 = pList1 = *ppList;

	for (i=0; i < iListLen1 - 1; i++)
	{
		pList2 = pList2->pNext;
	}

	pNext = pList2->pNext;
	pList2->pNext = NULL;
	pList2 = pNext;

	//now pList1 and pList2 point to totally separate lists
	MergeSortCommands(&pList1, iListLen1);
	MergeSortCommands(&pList2, iListLen2);

	if (StringComesAlphabeticallyBefore(pList1->pCommandName, pList2->pCommandName))
	{
		pMasterListHead = pMasterListTail = pList1;
		pList1 = pList1->pNext;
		pMasterListHead->pNext = NULL;
	}
	else
	{
		pMasterListHead = pMasterListTail = pList2;
		pList2 = pList2->pNext;
		pMasterListHead->pNext = NULL;
	}

	while (pList1 && pList2)
	{
		if (StringComesAlphabeticallyBefore(pList1->pCommandName, pList2->pCommandName))
		{
			pMasterListTail->pNext = pList1;
			pList1 = pList1->pNext;
			pMasterListTail->pNext->pNext = NULL;
			pMasterListTail = pMasterListTail->pNext;
		}
		else
		{
			pMasterListTail->pNext = pList2;
			pList2 = pList2->pNext;
			pMasterListTail->pNext->pNext = NULL;
			pMasterListTail = pMasterListTail->pNext;
		}
	}

	if (pList1)
	{
		pMasterListTail->pNext = pList1;
	}
	else
	{
		pMasterListTail->pNext = pList2;
	}

	*ppList = pMasterListHead;
}








	


void MasterWikiCommandCategory::SortCommands(void)
{
	int iCount = 0;
	SingleCommandStruct *pCounter = m_pFirstCommand;

	while (pCounter)
	{
		iCount++;
		pCounter = pCounter->pNext;
	}

	MergeSortCommands(&m_pFirstCommand, iCount);

}

void MasterWikiCommandCategory::WriteCommands(FILE *pOutFile)
{
	SingleCommandStruct *pCounter = m_pFirstCommand;

	while (pCounter)
	{
		fprintf(pOutFile, "%s\n\n", pCounter->pCommandDescription);

		pCounter = pCounter->pNext;
	}
}

char *MasterWikiCommandCategory::GetCategoryName()
{
	return m_CategoryName;
}



bool SourceParser::DoesVariableHaveValue(char *pVarName, char *pValue, bool bCheckFinalValueOnly)
{
	SourceParserVar *pVar = m_pFirstVar;


	while (pVar)
	{
		if (_stricmp(pVar->pVarName, pVarName) == 0)
		{
			if (bCheckFinalValueOnly)
			{
				int iLen = (int)strlen(pValue);
				if (strncmp(pValue, pVar->pValue + 1, iLen) == 0)
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			char tempBuffer[256];
			sprintf(tempBuffer, " %s ", pValue);
			if (strstri(pVar->pValue, tempBuffer))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		pVar = pVar->pNext;
	}

	return false;
}

void SourceParser::AddVariableValue(char *pVarName, char *pValue)
{
	SourceParserVar *pVar = m_pFirstVar;


	while (pVar)
	{
		if (_stricmp(pVar->pVarName, pVarName) == 0)
		{
			int iCurLen = (int)strlen(pVar->pValue);
			int iAddLen = (int)strlen(pValue);
			char *pNewBuf = new char[iCurLen + iAddLen + 2];
			sprintf(pNewBuf, " %s%s", pValue, pVar->pValue);
			delete pVar->pValue;
			pVar->pValue = pNewBuf;
			return;
		}
		
		pVar = pVar->pNext;
	}

	pVar = new SourceParserVar;

	pVar->pNext = m_pFirstVar;
	m_pFirstVar = pVar;

	pVar->pVarName = STRDUP(pVarName);
	int iCurLen = (int)strlen(pValue);
	pVar->pValue = new char[iCurLen + 3];
	sprintf(pVar->pValue, " %s ", pValue);
}

void SourceParser::SetVariablesFromTokenizer(Tokenizer *pTokenizer, char *pStartingDirectory)
{
	enumTokenType eType;
	Token token;
	char varName[256];

	while (1)
	{
		eType = pTokenizer->GetNextToken(&token);

		if (eType == TOKEN_NONE)
		{
			return;
		}

		pTokenizer->Assert(eType == TOKEN_IDENTIFIER, "Expected identifier name to set");
		pTokenizer->Assert(token.iVal < 255, "Var name overflow");

		if (_stricmp(token.sVal, "#include") == 0)
		{
			pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_STRING, 0, "Expected string after #include");
			char fullIncludeName[4096];
			if (strncmp(token.sVal, "..", 2) == 0)
			{
				sprintf(fullIncludeName, "%s%s", pStartingDirectory, token.sVal);
			}
			else
			{
				strcpy(fullIncludeName, token.sVal);
			}
			
			Tokenizer *pIncludeTokenizer = new Tokenizer;
			pIncludeTokenizer->SetExtraCharsAllowedInIdentifiers("#");
			if (!pIncludeTokenizer->LoadFromFile(fullIncludeName))
			{
				pTokenizer->Assertf(0, "Couldn't load include file %s", fullIncludeName);
			}


			char *pLastBackslash = strrchr(fullIncludeName, '\\');
			if (pLastBackslash)
			{
				*(pLastBackslash + 1) = 0;
			}

			SetVariablesFromTokenizer(pIncludeTokenizer, fullIncludeName);
		}
		else
		{
			strcpy(varName, token.sVal);

			pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_RESERVEDWORD, RW_EQUALS, "Expected = after var name");
			
			do
			{
				pTokenizer->AssertNextTokenTypeAndGet(&token, TOKEN_IDENTIFIER, 0, "expected identifier for var value");
				AddVariableValue(varName, token.sVal);

				pTokenizer->Assert2NextTokenTypesAndGet(&token, TOKEN_RESERVEDWORD, RW_COMMA, TOKEN_RESERVEDWORD, RW_SEMICOLON, "Expected , or ;");
			}
			while (token.iVal != RW_SEMICOLON);
		}
	} 
}

void SourceParser::FindVariablesFileAndLoadVariables(void)
{
	char directoryToTry[MAX_PATH];
	char fileToTry[MAX_PATH];

	strcpy(directoryToTry, m_ProjectPath);
	int iDirectoryStrLen = (int)strlen(directoryToTry);

	while (iDirectoryStrLen)
	{
		sprintf(fileToTry, "%sStructParserVars.txt", directoryToTry);
		Tokenizer *pTokenizer = new Tokenizer;
		pTokenizer->SetExtraCharsAllowedInIdentifiers("#");
		bool bSuccess = pTokenizer->LoadFromFile(fileToTry);

		if (bSuccess)
		{
			SetVariablesFromTokenizer(pTokenizer, directoryToTry);
			delete pTokenizer;
			return;
		}

		delete pTokenizer;
		directoryToTry[--iDirectoryStrLen] = 0;

		while (iDirectoryStrLen && directoryToTry[iDirectoryStrLen - 1] != '\\')
		{
			directoryToTry[--iDirectoryStrLen] = 0;
		}
	}
}

