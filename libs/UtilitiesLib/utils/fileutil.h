#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "file.h"

char * fileGetcwd(char * _DstBuf, int _SizeInBytes);

extern int quickload;
void fileScanAllDataDirs(const char* dir, FileScanProcessor processor);
int IsFileNewerThanDir(const char* dir, const char* filemask, const char* persistfile);
	// same as old textparser.ParserIsPersistNewer.  This checks a directory tree
	// for files that are newer than persistfile.  You can mask extensions
	// using filemask.  

//returns the complete pathname in long format
char *makeLongPathName_safe(char * anyPath, char *outBuff, int buffsize); //accepts both UNC and traditional paths, both long and short
#define makeLongPathName(anyPath, outBuff) makeLongPathName_safe( anyPath, outBuff, ARRAY_SIZE(outBuff) )

//Checks to see if a file has been updated.
//Also updates the age that this was last checked
int fileHasBeenUpdated(const char * fname, int *age);

int fileLineCount(const char *fname); // uses fgets, assumes line length < 64kB, assumes files ends with a newline

// Safely zips a file without allocating any memory or calling any of our functions (for use in minidump writing)
void fileZip(const char *filename);
void fileGZip(const char *filename);
void fileGunZip(const char *filename);

void fileWaitForExclusiveAccess(const char *filename);
bool fileCanGetExclusiveAccess(const char *filename);

bool fileAttemptNetworkReconnect(const char *filename);

#endif
