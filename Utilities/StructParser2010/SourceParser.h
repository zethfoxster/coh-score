#ifndef _SOURCEPARSER_H_
#define _SOURCEPARSER_H_

#include "tokenizer.h"

#include "FileListLoader.h"
#include "FileListWriter.h"

#include "IdentifierDictionary.h"
#include "SourceParserBaseClass.h"

#include "utils.h"

#include <vector>
#include <string>

#define MAX_BASE_SOURCE_PARSERS 8

#define MAX_MAGIC_WORDS_PER_BASE_SOURCE_PARSER 12

#define MAX_DEPENDENT_LIBRARIES 32

#define MAX_WIKI_PROJECTS 64
#define MAX_WIKI_CATEGORIES 256

typedef class AutoRunManager AutoRunManager;



class SourceParser
{
public:
	SourceParser();
	~SourceParser();

	void ParseSource(char *pProjectPath, char *pProjectFileName, char *pCurTarget, char *pCurConfiguration, char *pCurVCDir, char *pSolutionPath,
		char *pExecutable);

	void NukeCObjFile(char *pFileName);

	char *GetShortProjectName() { return m_ShortenedProjectFileName; }
	char *GetProjectPath() { return m_ProjectPath; }
	IdentifierDictionary *GetDictionary() { return &m_IdentifierDictionary; }

	void SetExtraDataFlagForFile(char *pFileName, int iFlag);

	int GetNumLibraries(void) { return m_iNumDependentLibraries; }
	char *GetNthLibraryName(int n) { return m_DependentLibraryNames[n]; }
	char *GetNthLibraryFullPath(int n) { return m_DependentLibraryFullPaths[n]; }
	bool IsNthLibraryXBoxExcluded(int n) { return m_bExcludeLibrariesFromXBOX[n]; }

	AutoRunManager *GetAutoRunManager() { return m_pAutoRunManager; }

	//returns true if the project is the game client, or a lib that is linked only into the game client
	bool ProjectIsClientOrClientOnlyLib(void);

	//returns true if the projet is the game server, or a lib that is linked only into the game server
	bool ProjectIsGameServerOrGameServerOnlyLib(void);

	//returns true if the project is an executable as opposed to a library
	bool ProjectIsExecutable(void) { return m_bIsAnExecutable; }

	int GetNumProjectFiles(void) { return m_iNumProjectFiles; }
	char *GetNthProjectFile(int n) { return m_ProjectFiles[n]; }

	bool DoesVariableHaveValue(char *pVarName, char *pValue, bool bCheckFinalValueOnly);


private://structs
	typedef struct SourceParserVar
	{
		char *pVarName;
		char *pValue;
		struct SourceParserVar *pNext;
	} SourceParserVar;


private:
	FileListLoader *m_pFileListLoader;
	FileListWriter *m_pFileListWriter;

	IdentifierDictionary m_IdentifierDictionary;

	int m_iNumSourceParsers;
	SourceParserBaseClass *m_pSourceParsers[MAX_BASE_SOURCE_PARSERS];
	AutoRunManager *m_pAutoRunManager;

	int m_iNumProjectFiles;
	char m_ProjectFiles[MAX_FILES_IN_PROJECT][MAX_PATH];
	bool m_bFilesNeedToBeUpdated[MAX_FILES_IN_PROJECT];

	int m_iNumDependencies[MAX_FILES_IN_PROJECT];
	int m_iDependencies[MAX_FILES_IN_PROJECT][MAX_DEPENDENCIES_SINGLE_FILE];

	int m_iExtraDataPerFile[MAX_FILES_IN_PROJECT];

	char m_FullProjectFileName[MAX_PATH];

	char m_Executable[MAX_PATH];

	char m_ProjectPath[MAX_PATH];
	char m_ShortenedProjectFileName[MAX_PATH];

	char m_SolutionPath[MAX_PATH];

	int m_iNumDependentLibraries;
	char m_DependentLibraryNames[MAX_DEPENDENT_LIBRARIES][MAX_PATH];
	char m_DependentLibraryFullPaths[MAX_DEPENDENT_LIBRARIES][MAX_PATH];
	bool m_bExcludeLibrariesFromXBOX[MAX_DEPENDENT_LIBRARIES];


	//esnure that the project contains the two master autogen files
	bool m_FoundAutoGenFile1;
	bool m_FoundAutoGenFile2;

	char m_AutoGenFile1Name[MAX_PATH];
	char m_AutoGenFile2Name[MAX_PATH];

	char m_SpecialAutoRunFuncName[MAX_PATH];

	//whether the project we're working on is an executable vs. a libary
	bool m_bIsAnExecutable;



	//---------------stuff used to do command-line compilation of auto-generated C files

	//stuff passed in on the command line
	char *m_pCurTarget;
	char *m_pCurConfiguration;
	char *m_pCurVCDir;

	//stuff ripped out of vcproj file
	char m_AdditionalIncludeDirs[TOKENIZER_MAX_STRING_LENGTH];
	char m_PreprocessorDefines[TOKENIZER_MAX_STRING_LENGTH];
	char m_ObjectFileDir[MAX_PATH];

	//stuff used to check whether we need to C file compiling
	bool m_bCleanBuildHappened;
	bool m_bProjectFileChanged;

	SourceParserVar *m_pFirstVar;

private:
	void AddProjectFiles(const std::vector<std::string> &attributes);
	void ProcessProjectFile();
	bool NeedToUpdateFile(char *pFileName, int iExtraData,  bool bForceUpdateUnlessFileDoesntExist);
	void ScanSourceFile(char *pSourceFile);
	
	void LoadSavedDependenciesAndRemoveObsoleteFiles(void);

	//returns true if at least one file was set to udpate that was previously not set to update
	//
	//find all need-to-update files which have dependencies, and set all the other
	//files they are dependent on to be need-to-update, and recurse. 
	bool ProcessAllLoadedDependencies();
	void ClearAllDependenciesForUpdatingFiles(void);
	void AddDependency(int iFile1, int iFile2);
	void ProcessAllFiles_ReadAll();
	void ProcessAllFiles();
	int FindProjectFileIndex(char *pFileName);
	void DestroyLegacyMasterFiles(bool bBuildAll);
	void MakeAutoGenDirectory();
	void ProcessSolutionFile();
	void CheckForRequiredFiles(const char *pFileName);
	bool IsLibraryXBoxExcluded(char *pLibName);
	bool DoMasterFilesExist();
	bool DidCleanBuildJustHappen();
	void CleanOutAllAutoGenFiles();
	bool IsQuickExitPossible();
	void CreateCleanBuildMarkerFile();
	void CreateParsers(void);
	int GetSVNVersion(char *pFileName);
	bool MakeSpecialAutoRunFunction(void);

	void AddVariableValue(char *pVarName, char *pValue);
	void SetVariablesFromTokenizer(Tokenizer *pTokenizer, char *pStartingDirectory);
	void FindVariablesFileAndLoadVariables(void);
};


//global TRACE for verbose stuff
extern int gVerbose;
#define TRACE(...)  {if (gVerbose) {printf(__VA_ARGS__); fflush(stdout);}}

#define GENERATE_FAKE_DEPENDENCIES 1
#endif
