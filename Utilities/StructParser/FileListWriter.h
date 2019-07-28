#ifndef _FILELISTWRITER_H_
#define _FILELISTWRITER_H_

#include "windows.h"
#include "stdio.h"
#include "filelistloader.h"

class FileListWriter
{
public:
	FileListWriter();
	~FileListWriter();

	void OpenFile(char *pListFileName, char *pObjFileDirName);
	void AddFile(char *pFileName, int iExtraData, int iNumDependencies, int iDependencies[MAX_DEPENDENCIES_SINGLE_FILE]);
	void CloseFile();

private:
	FILE *m_pOutFile;
};


#endif