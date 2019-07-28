#ifndef _FILELISTLOADER_H_
#define _FILELISTLOADER_H_

#include "windows.h"

#define MAX_FILES_IN_PROJECT 2048
 
#define MAX_DEPENDENCIES_SINGLE_FILE 64

class FileListLoader
{
public:
	FileListLoader();
	~FileListLoader();

	bool LoadFileList(char *pListFileName);
	
	FILETIME *GetMasterFileTime() { return &m_MasterFileTime; }
	bool IsFileInList(char *pFileName);

	int GetNumFiles() { return m_iNumFiles; }
	char *GetNthFileName(int n) { return m_FileNames[n];}
	int GetNumDependencies(int n) { return m_iNumDependencies[n]; }
	int GetNthDependency(int iFileNum, int n) { return m_iDependencies[iFileNum][n]; }
	int GetExtraData(int iFileNum) { return m_iExtraData[iFileNum]; }
	FILETIME *GetNthFileTime(int n) { return &m_FileTimes[n]; }
	bool GetNthFileExists(int n) { return m_FileExists[n]; }

	//we specifically save the obj file directory so that we can quickly check it to check for clean
	//builds without having to parse the vcproj file, so that quick exits (when nothing has changed)
	//can be as fast as possible
	char *GetObjFileDirectory(void) { return m_ObjFileDirectory; }

private:
	FILETIME m_MasterFileTime;
	int m_iNumFiles;
	char m_FileNames[MAX_FILES_IN_PROJECT][MAX_PATH];

	int m_iExtraData[MAX_FILES_IN_PROJECT];

	FILETIME m_FileTimes[MAX_FILES_IN_PROJECT];
	bool m_FileExists[MAX_FILES_IN_PROJECT];


	int m_iNumDependencies[MAX_FILES_IN_PROJECT];
	int m_iDependencies[MAX_FILES_IN_PROJECT][MAX_DEPENDENCIES_SINGLE_FILE];

	char m_ObjFileDirectory[MAX_PATH];
};

#endif

