#include "Stdio.h"
#include "stdarg.h"
#include "windows.h"
#include "tokenizer.h"

bool gbLastFWCloseActuallyWrote = false;

typedef struct FileWrapper
{
	FILE *pFile;
	bool bWriting;
	char *pNewContents;
	int iNewContentsSize;
	int iNewContentsBuffSize;

	char *pOriginalContents;
	char filename[MAX_PATH];

} FileWrapper;

FileWrapper *fw_fopen(const char *pFilename, const char *pMode)
{
	if (strchr(pMode, 'w') || strchr(pMode, 'W'))
	{
		FILE *pFile = fopen(pFilename, "rt");
		char *pOriginalContents = NULL;
		if (pFile)
		{
			fseek(pFile, 0, SEEK_END);

			//allocate extra space because rt vs rb means there are \n inconsistencies
			int iFileSize = ftell(pFile) * 2;

			fseek(pFile, 0, SEEK_SET);

			pOriginalContents = new char[iFileSize + 1];
			memset(pOriginalContents, 0, iFileSize + 1);

			int iBytesRead = (int)fread(pOriginalContents, 1, iFileSize, pFile);

			pOriginalContents[iBytesRead] = 0;
	
			fclose(pFile);
	}



		FileWrapper *pWrapper = new FileWrapper;
		pWrapper->bWriting = true;
		pWrapper->pOriginalContents = pOriginalContents;
		pWrapper->pNewContents = new char[32];
		memset(pWrapper->pNewContents, 0, 32);
		pWrapper->iNewContentsBuffSize = 32;
		pWrapper->iNewContentsSize = 0;
		pWrapper->pFile = NULL;
		strcpy(pWrapper->filename, pFilename);

		return pWrapper;

	}
	else
	{

		FILE *pFile = fopen(pFilename, pMode);

		if (pFile)
		{
			FileWrapper *pWrapper = new FileWrapper;
			pWrapper->bWriting = false;
			pWrapper->pOriginalContents = NULL;
			pWrapper->pFile = pFile;
			pWrapper->pNewContents = NULL;
			return pWrapper;
		}
	
	}

	return NULL;
}
void DeleteWrapper(FileWrapper *pFile)
{
	if (pFile->pNewContents)
	{
		delete pFile->pNewContents;
	}

	if (pFile->pOriginalContents)
	{
		delete pFile->pOriginalContents;
	}

	delete pFile;
}

void fw_fclose(FileWrapper *pFile)
{
	gbLastFWCloseActuallyWrote = false;

	if (pFile->bWriting)
	{
		if (pFile->pOriginalContents && 
			strcmp(pFile->pOriginalContents, pFile->pNewContents) == 0)
		{
			DeleteWrapper(pFile);
			return;
		}


		FILE *pOutFile = fopen(pFile->filename, "wt");

		if (!pOutFile)
		{
			Tokenizer::StaticAssertf(0, "Could not open %s for writing", pFile->filename);
		}

		fprintf(pOutFile, "%s", pFile->pNewContents);

		fclose(pOutFile);

		DeleteWrapper(pFile);

		gbLastFWCloseActuallyWrote = true;
	}
	else
	{
		fclose(pFile->pFile);
		DeleteWrapper(pFile);
	}
}

void WriteBytes(FileWrapper *pFile, char *pBytes, int iNumBytes)
{
	if (!iNumBytes)
	{
		return;
	}

	int iNewSize = pFile->iNewContentsSize + iNumBytes;

	if (iNewSize < pFile->iNewContentsBuffSize - 1)
	{
		memcpy(pFile->pNewContents + pFile->iNewContentsSize, pBytes, iNumBytes);
		pFile->iNewContentsSize += iNumBytes;

		return;
	}

	int iNewBuffSize = pFile->iNewContentsBuffSize * 2;

	while (iNewSize >= iNewBuffSize - 1)
	{
		iNewBuffSize *= 2;
	}

	char *pNewBuff = new char[iNewBuffSize];
	memset(pNewBuff, 0, iNewBuffSize);
	if (pFile->iNewContentsSize)
	{
		memcpy(pNewBuff, pFile->pNewContents, pFile->iNewContentsSize);
	}

	delete pFile->pNewContents;

	pFile->pNewContents = pNewBuff;
	pFile->iNewContentsBuffSize = iNewBuffSize;

	memcpy(pFile->pNewContents + pFile->iNewContentsSize, pBytes, iNumBytes);
	pFile->iNewContentsSize += iNumBytes;
}


int fw_fseek(FileWrapper *pFile, long iBytes, int iWhere)
{
	Tokenizer::StaticAssert(!pFile->bWriting, "can't fseek a write file");

	return fseek(pFile->pFile, iBytes, iWhere);
}
int fw_fprintf(FileWrapper *pFile, const char *pFmt, ...)
{
	char buffer[30000];
	int iLen;

	Tokenizer::StaticAssert(pFile->bWriting, "can't fprintf a read file");


	va_list ap;

	va_start(ap, pFmt);

	iLen = vsprintf(buffer, pFmt, ap);
	
	Tokenizer::StaticAssert(iLen < 30000, "buffer overflow in fw_fprintf");

	va_end(ap);

	WriteBytes(pFile, buffer, iLen);

	return iLen;
}

char fw_getc(FileWrapper *pFile)
{
	Tokenizer::StaticAssert(!pFile->bWriting, "can't getc a write file");
	return getc(pFile->pFile);
}

void fw_putc(char c, FileWrapper *pFile)
{	
	Tokenizer::StaticAssert(pFile->bWriting, "can't putc a read file");
	WriteBytes(pFile, &c, 1);

}

int fw_ftell(FileWrapper *pFile)
{
	Tokenizer::StaticAssert(!pFile->bWriting, "can't ftell a write file");

	return ftell(pFile->pFile);
}

int fw_fread(void *pBuf, int iSize, int iCount, FileWrapper *pFile)
{
	Tokenizer::StaticAssert(!pFile->bWriting, "can't fread a write file");

	return (int)fread(pBuf, iSize, iCount, pFile->pFile);
}



bool FileExists(const char *pFileName)
{
	FILE *pFile = fopen(pFileName, "rb");

	if (pFile)
	{
		fclose(pFile);
		return true;
	}

	return false;
}