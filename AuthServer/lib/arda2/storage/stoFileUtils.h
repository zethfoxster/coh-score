/**
    purpose:    Utility functions for OS-neutral file/directory info functions
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#ifndef   INCLUDED_stoFileUtils
#define   INCLUDED_stoFileUtils
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlVector.h"
#include "arda2/core/corStdString.h"
#include "arda2/core/corError.h"
#include <fcntl.h>

// compatability defines and includes
#if CORE_SYSTEM_WINAPI
#include <sys/stat.h>
#include <io.h>
#include <direct.h>
#define _seek_offset long

#if CORE_COMPILER_MSVC8 || CORE_SYSTEM_XENON
#include <errno.h>
#endif

#elif CORE_SYSTEM_PS3

#include <unistd.h>
#define _open   open
#define _close  close
#define _lseek  lseek
#define _seek_offset off_t
#define _read   read
#define _write  write
#define _mkdir  cellFsMkdir
#define mkdir   cellFsMkdir
#define _rmdir  cellFsRmdir
#define rmdir   cellFsRmdir
typedef uint mode_t;
#define _O_CREAT    O_CREAT
#define _O_TRUNC    O_TRUNC
#define _S_IREAD    0x0000400
#define _S_IWRITE   0x0000200
#define _O_RDONLY   O_RDONLY
#define _O_WRONLY   O_WRONLY
#define _O_RDWR     O_RDWR
#define _O_BINARY   0

inline char* getcwd(char* buffer, int) { return strncpy(buffer,".\n",2); }
inline int mkstemp(char* name) { return open(name,O_CREAT|O_RDWR);}

#else

#include "ctype.h"
#include <sys/stat.h>
#include <unistd.h>
#define _open open
#define _close close
#define _lseek lseek
#define _seek_offset off_t
#define _read read
#define _write write
#define _fstat fstat
#define _stat stat
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_BINARY 0

#include <errno.h>
#include <dirent.h>

#endif // CORE_SYSTEM_WINAPI


namespace stoFileUtils
{
    // OS independent Directory operations
    bool    FileExists( IN const char *fullpath );

	bool    IsReadOnly( IN const char *fullpath );

    bool    IsFile( IN const char *fullPath );

    bool    IsDirectory( IN const char *fullPath );

    bool    CreateDirectory( IN const char *fullPath, IN bool force=true );

    bool    DeleteDirectory( IN const char *fullPath, IN bool force=true );

    void    ListDirectory( IN const char *fullPath, OUT std::vector<std::string> &results );

    bool    CreateDirectoryTree( IN const char *path, bool bForce = true );

    bool    CreateDirectoryForFile( IN const char *path, bool bForce = true );

    DEPRECATED bool CreateDirectoryPath( IN const char *fullPath, IN bool force=false);
    inline bool CreateDirectoryPath( IN const char *fullPath, IN bool force)
    {
        return CreateDirectoryForFile(fullPath, force);
    }

    bool    DeleteFile( IN const char *fullPath, IN bool force = true );

    int32   GetFileModifiedTime( IN const char *fullPath );

    int32   GetFileSize( IN const char *fullPath );

    bool    MoveFile( IN const char *sourcePath, IN const char *destinationPath);

    bool    CopyFile( IN const char *sourcePath, IN const char *destinationPath);

    bool    Match( IN const char *fileName, IN const char *pattern);

    void    AddDefaultExtension(IN const char *originalName, IN const char *extension, 
        OUT char *outName);

    void SplitPath( IN const char *path, OUT char *drive, OUT char *dir, 
        OUT char *fname, OUT char *ext );

    void MergePath( OUT char *path, IN const char *drive, IN const char *dir, 
        IN const char *fname, IN const char *ext );

    errResult BuildRecursiveFileList(const char *dirName, std::vector<std::string> &output);
    errResult BuildRecursiveFileListWithExclude(const char *dirName, std::vector<std::string> &output, 
                                                                     std::vector<std::string> &excludeDirs);

    std::string GetExecutableDirectory();

    errResult GetTempFileName(const char *directory, const char *prefix, OUT std::string &name);

};


#endif // INCLUDED_stoFileUtils
