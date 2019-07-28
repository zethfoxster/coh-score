/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 */

#ifndef _FILE_H
#define _FILE_H

C_DECLARATIONS_BEGIN

#include <stdio.h>
#include <io.h>
#include "stdtypes.h"
#include "memcheck.h"


#define sfread fread
#define fileClose fclose

typedef struct FileWrapper FileWrapper;
#define FILE FileWrapper
#undef getc
#undef ferror
#undef fopen // defined in stdtypes.h to be "please include file.h"
#define getc(f) x_getc(f)
#define fseek(f, dist, whence) x_fseek(f, dist, whence)
#define fopen(name, how) x_fopen(name, how)
#define fclose(f) x_fclose(f)
#define ftell(f) x_ftell(f)
#define fread(buf, size, count, f) x_fread(buf, size, count, f)
#define fwrite(buf, size, count, f) x_fwrite(buf, size, count, f)
#define fgets(buf, size, f) x_fgets(buf, size, f)
#define vfprintf x_vfprintf
#define fprintf x_fprintf
#define ferror x_ferror
#define fflush(f) x_fflush(f)
// #define fscanf x_fscanf
#define fgetc(f) x_fgetc(f)
#define fputc(c, f) x_fputc(c, f)
#define setvbuf(f, buf, mode, size) x_setvbuf(f, buf, mode, size)

#define FW_ALLOC_STACK_SIZE 4096

typedef enum {
	IO_CRT,
	IO_WINIO,
	IO_GZ,
	IO_PIG,
	IO_STRING,
} IOMode;

typedef struct FileWrapper
{
	void	*fptr;
	IOMode	iomode;
	const char	*nameptr;
	U32	nameptr_needs_freeing:1;
#ifdef FW_TRACK_ALLOCATION
	char	allocStack[FW_ALLOC_STACK_SIZE];	// Who allocated me?
#endif
} FileWrapper;

#define fileAlloc(fname,lenp) fileAlloc_dbg(fname,lenp MEM_DBG_PARMS_INIT)
#define fileAllocEx(fname,lenp,callback) fileAllocEx_dbg(fname,lenp,"rb",callback MEM_DBG_PARMS_INIT)

#define fa_read(dst, src, bytes) memcpy(dst, src, bytes); ((char*)(src))+=bytes;

typedef enum{
	FSA_NO_EXPLORE_DIRECTORY,
	FSA_EXPLORE_DIRECTORY,
	FSA_STOP
} FileScanAction;
typedef FileScanAction (*FileScanProcessor)(char* dir, struct _finddata32_t* data);

typedef struct FileScanContext FileScanContext;
typedef FileScanAction (*FileScanProcessorEx)(FileScanContext* context);
struct FileScanContext{
	FileScanProcessorEx processor;
	char* dir;
	struct _finddata32_t* fileInfo;
	void* userData;
};
typedef int (*FileProcessCallback)(int bytes_processed, int total); // Return 0 to cancel reading/writing

typedef struct StuffBuff StuffBuff;
typedef struct _SYSTEMTIME SYSTEMTIME;

typedef enum{
	FSF_FILES=0x01,
	FSF_FOLDERS=0x02,
	FSF_FILES_AND_FOLDERS=0x03, // = FSF_FILES | FSF_FOLDERS
	FSF_UNDERSCORED=0x04, // 0x04 include files begining with an underscore (or this with the above choices)
	FSF_NOHIDDEN=0x08, // Excludes hidden and system files
} FileScanFolders;

typedef void (*FileExtraLoggingFunction)(const char *file);

#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8


bool fileInitSys(); // fileInitSys MUST be called before any other file functions
void fileInitCache(); // Loads piggs and scans data dir
void fileAddDataDir(const char* aPath);
const char *fileBaseDir(); // root directory that the game lives in
const char *fileDataDir(); // data driectory, will be basedir/data
const char *filePiggDir(); // pigg driectory, will be basedir/piggs
void fileSetBaseDir(const char *dir); // override base directory, use before fileInitSys
void fileSetDataDir(const char *dir); // override data directory, use before fileInitCache
void fileSetPiggDir(const char *dir); // override pigg directory, use before fileInitCache
int fileUseLegacyLayout(int enabled); // use old path layout
int fileLegacyLayout(); // is using old path layout
char *fileMakePath(const char *filename, const char *subdir, const char *legacysubdir, bool makedirs); // legacysubdir can be NULL for default dir
#define fileDebugPath(fn) fileMakePath(fn, "debug", "debug", true)
char *fileTempName();
int fileNewer(const char *refname, const char *testname);
int fileNewerOrEqual(const char *refname, const char *testname);
__time32_t fileLastChanged(const char *refname); // this is local time
__time32_t fileLastChangedAltStat(const char *refname); // this is local time (drill into it to see why it's not really UTC)
bool fileLastChangedWindows(const char *refname, SYSTEMTIME *systemtime); // this one is UTC
bool fileDatesEqual(__time32_t date1, __time32_t date2, bool bAllowDSTError);
char *fileFixSlashes(char *str);
char* fileFixName(char* src, char* tgt);
char *fileLocateWrite_s(const char *fname, char *dest, size_t dest_size); // dest can be NULL, then new memory is allocated, make sure to free it with fileLocateFree()!
#define fileLocateWrite(fname, dest) fileLocateWrite_s(fname, SAFESTR(dest))
char *fileLocateRead_s(const char *fname, char *dest, size_t dest_size);
#define fileLocateRead(fname, dest) fileLocateRead_s(fname, SAFESTR(dest))
void fileLocateFree(char *data); // Frees a pointer that was returned from fileLocateX()
FILE *fileOpen(const char *fname,char *how);
FILE *fileOpenEx(FORMAT fnameFormat, char *how, ...);
FILE *fileOpenStuffBuff(StuffBuff *sb);
void fileFree(void *mem);
char **fileScanDir(char *dir,int *count_ptr);
char **fileScanDirFolders(char *dir, int *count_ptr, FileScanFolders folders);
void fileScanDirFreeNames(char **names,int count);
void fileScanDirRecurseEx(const char* dir, FileScanProcessor processor);
void fileScanDirRecurseContext(const char* dir, FileScanContext* context);
FileScanAction printAllFileNames(char* dir, struct _finddata32_t* data); // Sample FileScanProcessor
int printPercentage(int bytes_read, int total); // Sample FileProcessCallback
void *fileAllocEx_dbg(const char *fname,int *lenp, const char* mode, FileProcessCallback callback MEM_DBG_PARMS);
void *fileAlloc_dbg(const char *fname,int *lenp MEM_DBG_PARMS);
int fileRenameToBak(const char *fname); // returns 0 on success
int fileForceRemove(const char *fname); // removes read-only files, returns -1 on failure
int fileMoveToRecycleBin(const char *filename); // removes a file and moves it to the recycle bin; returns 0 on success
int fileCopy(char *src, char *dst); // Copy a file (with timestamps); returns 0 upon success
int fileMove(char *src, char *dst);
int fileMakeLocalBackup(const char *_fname, int time_to_keep); // Make an incremental backup
char *fileMakeBackupName(char *out, int out_len, char *stem, int version);
bool fileExponentialBackup( char *fn, U32 intervalSeconds ); // make a numbered backup where the time between backups doubles at intervals.
bool fileMoveToBackup(char *fn);
void fileOpenWithEditor(const char *localfname);
		

intptr_t fileSize(const char *fname);
S64 fileSize64(const char *fname); // Slow!  Does not use FileWatcher.
int fileExists(const char *fname);
bool wfileExists(const wchar_t *fname);
int dirExists(const char *dirname);
int filePathCompare(const char* path1, const char* path2);
int filePathBeginsWith(const char* fullPath, const char* prefix);
U64 fileGetFreeDiskSpace(const char* fullPath);
void fileAllPathsAbsolute(bool newvalue);
int fileIsAbsolutePath(const char *path);
void fileFreeOldZippedBuffers(void); // Frees buffers on handles that haven't been accessed in a long time
void fileFreeZippedBuffer(FileWrapper *fw); // Frees a buffer, if there is one, on a zipped pigged file handle

#ifndef FINAL
int fileIsUsingDevData(void);
int isDevelopmentMode(void); // Returns 1 if we're in development mode, 0 if in production
int isDevelopmentOrQAMode(void); // Returns 1 if we're in development or QA mode, 0 if in production
int isProductionMode(void); // Returns 0 if we're in development mode, 1 if in production
int debugFilesEnabled(void); // Returns 1 if extra debug file creation is enabled
int enableDebugFiles(int); // Enables or disables extra debug file creation
#else
static INLINEDBG int fileIsUsingDevData(void) { return 0; }
static INLINEDBG int isDevelopmentMode(void) { return 0; }
static INLINEDBG int isDevelopmentOrQAMode(void) { return 0; }
static INLINEDBG int isProductionMode(void) { return 1; }
static INLINEDBG int debugFilesEnabled(void) { return 0; }
static INLINEDBG int enableDebugFiles(int x) { return 0; }
#endif

void fileDisableWinIO(int disable); // Disables use of Windows ReadFile/CreateFile instead of CRT for binary files

void *fopenf(FORMAT name, const char *how, ...);
void *x_fopen(const char *name,const char *how);
int x_fclose(FileWrapper *fw);
S64 x_fseek(FileWrapper *fw, S64 dist, int whence);
int x_getc(FileWrapper *fw);
S64 x_ftell(FileWrapper *fw);
intptr_t x_fread(void *buf,size_t size1,size_t size2,FileWrapper *fw);
intptr_t x_fwrite(const void *buf,size_t size1,size_t size2,FileWrapper *fw);
char *x_fgets(char *buf,int len,FileWrapper *fw);
int x_fflush(FileWrapper *fw);
int x_vfprintf(FileWrapper *fw, const char *format, va_list va);
int x_fprintf (FileWrapper *fw, FORMAT format, ...);
int x_ferror(FileWrapper* fw);
// i'm assuming some files were using fscanf and fopen and weren't including file.h
// so this never got commented out
// int x_fscanf(FileWrapper* fw, const char* format, ...);
int x_fgetc(FileWrapper* fw);
int x_fputc(int c,FileWrapper* fw);
int x_setvbuf(FileWrapper* fw, char *buffer, int mode, size_t size);
int x_flock(FileWrapper* fw, int mode);
int x_fsync(FileWrapper* fw);
FileWrapper *fileWrap(void *real_file_pointer);

FILE *fileGetStdout();
void *fileRealPointer(FileWrapper *fw);

void *fileLockRealPointer(FileWrapper *fw); // This implements support for Pig files, and must be followed by an unlock when done
void fileUnlockRealPointer(FileWrapper *fw);

int fileTruncate(FileWrapper *fw, U64 newsize); // Returns 0 on success

int fileGetSize(FILE* file);
int fileSetSize(char* filename, int newSize);
void fileSetTimestamp(char* fullFilename, int timestamp);
int fileSetLogging(int on);
void fileSetExtraLogging(FileExtraLoggingFunction function);

bool fileChecksum(char *fn, U32 checksum[4]);
bool fileWriteChecksum(char *fn);
bool fileChecksumfileMatches(char *fn);
int fileRemoveChecksum(char *fn);
int fileMoveChecksum(char *src, char *dst);

int fileCheckForNewPiggs();

C_DECLARATIONS_END

#endif
