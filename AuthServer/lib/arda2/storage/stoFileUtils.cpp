/**
    purpose:    Utility functions for OS-neutral file/directory info functions
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    modified:   $DateTime: 2007/12/12 10:43:51 $
                $Author: pflesher $
                $Change: 700168 $
                @file
**/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFileUtils.h"

using namespace std;

#if CORE_SYSTEM_WINAPI 
static const char gDirSeps[] = "/\\";
#else
static const char gDirSeps[] = "/";
#endif


// helper functions

/**
 *  Finds the first occurance of szSearchString within szString. The search
 *  is case-insensitive
 *
 *  @param  szString        the string to search within
 *  @param  szSearchString  the string to search for
 *   
 *  @return pointer to the location of the found string, or NULL if not found
 */
static const char* FindString( const char* szString, const char* szSearchString )
{
    const char* pStart = szString;
    const char* pEnd = szString + strlen(szString) - strlen(szSearchString) + 1;

    while ( pStart < pEnd )
    {
        for ( int i = 0; ; ++i )
        {
            if ( szSearchString[i] == '\0' )
                return pStart;

            if ( tolower(pStart[i]) != tolower(szSearchString[i]) )
                break;
        }
        ++pStart;
    }
    return NULL;
}


/**
 *  Finds the next occurance of a character from the given set in the passed string.
 *  This works like strcspn(), but returns pointers instead of offsets
 *  @param  szString - the string to search within
 *  @param  szSearchChars - set of characters to search for
 *   
 *  @return pointer to the location within szString of the first found char (including
 *  NULL terminator char)
 */
static const char* NextCharInString( const char *szString, const char* szSearchChars )
{
    while ( *szString )
    {
        const char* pSearchChar = szSearchChars;
        while ( *pSearchChar )
        {
            if ( *szString == *pSearchChar )
                return szString;

            ++pSearchChar;
        }
        ++szString;
    }
    return szString;
}


/**
 *   Check if file/directory exists
 *
 *  @param path - full path to the file/directory to check
 * 
 *  @return true is the file/directory exists
 */
bool stoFileUtils::FileExists( IN const char *path )
{
#if CORE_SYSTEM_PS3
    struct CellFsStat statResult;
    return ( cellFsStat(path, &statResult)==CELL_FS_SUCCEEDED );
#else
    return access(path, 0) == 0;
#endif
}



/**
 *   Check if this path is a file, NOT a directory
 *
 *  @param path - full path to the file/directory to check
 * 
 *  @return true is the path is a file
 */
bool stoFileUtils::IsFile( IN const char *path )
{
    // NOTE: The function "stat" fails if the path has a trailing slash.
    //       Remove trailing slash if there is one.

    char tmp[_MAX_PATH];
    strcpy(tmp, path);

    int len = (int)strlen(tmp);
    if (tmp[len-1] == '\\' || tmp[len-1] == '/')
        tmp[len-1] = '\0';

#if CORE_SYSTEM_PS3
    int hFile = open(tmp,O_RDONLY);
    struct CellFsDirent dirResult;
    size_t size;
    CellFsErrno err = cellFsReaddir(hFile, &dirResult, &size);
    close(hFile);
    if (err==CELL_FS_SUCCEEDED )
        return (dirResult.d_type == CELL_FS_TYPE_REGULAR);
    else
        return false;
#else
    struct stat buf;
    if ( stat(tmp, &buf) != 0 )
        return false;

    return (buf.st_mode & S_IFREG) != 0;
#endif
}


/**
 *   Check if this path is a directory.
 *
 *  @param path - full path to the file/directory to check
 * 
 *  @return true is the path is a directory
 */
bool stoFileUtils::IsDirectory( IN const char *path )
{
    if (!FileExists(path))
        return false;

    return !IsFile(path);
}


/**
 * Determine if the file system object is read only
 *
 * @param path  The full path to the object in question.
 * @return      Returns true if the object is read only. 
 *              Returns false if the object is writable, 
 *              or if it doesn't exist
 */
bool stoFileUtils::IsReadOnly( IN const char *path )
{
#if CORE_SYSTEM_PS3
    // As of 0_5_13 st_mode is undefined in PS3 CELL OS
    struct CellFsStat statResult;
    if (cellFsStat(path, &statResult)==CELL_FS_SUCCEEDED)
        return (statResult.st_mode & _S_IWRITE) == 0;
#else
    struct stat buf;
    if ( stat(path, &buf) == 0 )
        return (buf.st_mode & _S_IWRITE) == 0;
#endif
    return false;
}
/**
    Return the contents of a directory in a platform independant manner.

    @param pathspec  Full path to the directory to get. This is not recursive.
                    May contain wildcard characters for file name, i.e.
                    "c:\temp\*.bmp".
 
  @return entries - a vector of strings, one for each entry in the directory
*/
void stoFileUtils::ListDirectory( IN const char *pathSpec, OUT vector<string> &results )
{
#if CORE_SYSTEM_WINAPI

    string sPathSpec(pathSpec);
    if ( stoFileUtils::IsDirectory(sPathSpec.c_str()) )
        sPathSpec += "/*";

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    hFind = FindFirstFile(sPathSpec.c_str(), &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        //printf ("Directory: <%s> doesn't exist.\n", path);
        return;
    }

    //iterate through files and directories
    int alive = true;
    while (alive)
    {
        // skip "." and ".."
        if (strcmp(FindFileData.cFileName,".") != 0 && strcmp(FindFileData.cFileName,"..") != 0)
        {
            results.push_back( string(FindFileData.cFileName));
        }
        alive = FindNextFile(hFind, &FindFileData);
    }

    FindClose(hFind);

#else

    char *fileSpec = NULL;
    char allFilesString[2];
    sprintf(allFilesString, "*");
    char pathSpecCopy[_MAX_PATH];
    strcpy(pathSpecCopy, pathSpec);

    if ( stoFileUtils::IsDirectory(pathSpec) )
    {
        fileSpec = allFilesString;
    }
    else
    {
        fileSpec = strrchr(pathSpecCopy, '/');
        if (!fileSpec)
            fileSpec = strrchr(pathSpecCopy, '\\');

        if (fileSpec)
        {
            // found directory/pathname, separate them with a NULL
            *fileSpec = '\0';
            fileSpec++;
        } else {
            // just a filename?? can't list this
            return;
        }
    }

#if CORE_SYSTEM_PS3
    int hDir;
    if (cellFsOpendir(pathSpecCopy,&hDir)==CELL_FS_SUCCEEDED)
    {
        CellFsDirent info;
        size_t size;
        while ( cellFsReaddir(hDir,&info,&size)==CELL_FS_SUCCEEDED )
        {
            // skip "." and ".."
            if ( strcmp(info.d_name, ".") == 0 || strcmp(info.d_name, "..") == 0 )
                continue;

            if ( Match(info.d_name, fileSpec) )
            {
                results.push_back( string(info.d_name) );
                //printf("Found entry: %s\n", info.d_name);
            }
        }
    }

#else

    DIR *d = opendir(pathSpecCopy);
    if (d)
    {
        struct dirent *info;
        while ( (info=readdir(d)) )
        {
            // skip "." and ".."
            if ( strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0 )
                continue;

            if ( Match(info->d_name, fileSpec) )
            {
                results.push_back( string(info->d_name) );
                //printf("Found entry: %s\n", info->d_name);
            }


        }
    }

#endif // else CORE_SYSTEM_PS3

#endif
}


/**
 *   create a directory.
 *
 *  @param path - full path to the file/directory to create
 * 
 *  @return true if the directory is created
 */
bool stoFileUtils::CreateDirectory( IN const char *path, IN bool force )
{
    if (force)
    {
        //printf("createdir force = %d\n", force);
        if (IsDirectory(path) )
        {
            DeleteDirectory(path);
        }
        else if (IsFile(path) )
        {
            DeleteFile(path);
        }
    }

#if CORE_SYSTEM_WINAPI
    if (!::CreateDirectory(path, NULL) )
        return false;
#else
    //LINUX create directory goes here
    mode_t m = (mode_t)0777;
    if (mkdir(path, m) == -1)
    {
        printf("Error: Unable to create directory %s: %d\n", path, errno);
        return false;
    }
    else
    {
        //printf("Created directory %s mode %d.\n", path, m);
    }
#endif

    return true;
}


/**
 *   delete a directory.
 *
 *  @param path - full path to the file/directory to create
 * 
 *  @return true if the directory is created
 */
bool stoFileUtils::DeleteDirectory( IN const char *path, IN bool force )
{
    //printf("delete dir %s %d\n", path, force);

    if (!force)
    {
        if (rmdir(path) == 0)
            return true;
        else
            return false;
    }

    // recurse through directories and remove everything.
    vector<string> entries;
    ListDirectory(path, entries);
    vector<string>::iterator it = entries.begin();
    vector<string>::iterator end = entries.end();

    for (; it != end; it++)
    {
        string full = string(path) + "/" + it->c_str();
        //printf("iteraing for %s isdir = %d\n", full.c_str(), IsDirectory(full.c_str()) );
        if (IsDirectory(full.c_str()))
        {
            if (!DeleteDirectory(full.c_str(), force) )
            {
                printf("Delete directory failed for: %s %d\n", full.c_str(), errno);
            }
        }
        else 
        {

            if (!DeleteFile(full.c_str(), force) )
            {
                printf("Delete file failed for: %s %d\n", it->c_str(), errno);
            }
        }
    }

    return rmdir(path) == 0;
}


/**
 *   Create all of the directories in the specified path.
 *
 *  @param path - full path with multiple directories (root/dir1/dir2/dir3)
 * 
 *  @return true iif the directories were all created
 */
bool stoFileUtils::CreateDirectoryTree( IN const char *path, IN bool force )
{
    if ( *path == '\0' )
        return true;

    char incremental[_MAX_PATH];
    const char* pLastSep = path;
    incremental[0] = '\0';

    // if a drive is included, we need to skip past it
    if ( isalpha(path[0]) && path[1] == ':' )
    {
        pLastSep += 2;
        strncat(incremental, path, 2);
        if ( path[2] == '\0' )
            return true;
    }

    do 
    {
        // stop processing if we've reached a trailing separator
        if ( strchr(gDirSeps, *pLastSep) != 0 && pLastSep[1] == '\0' )
            break;

        const char* pNextSep = NextCharInString(pLastSep + 1, gDirSeps);
        strncat(incremental, pLastSep, pNextSep - pLastSep);

        if ( !stoFileUtils::IsDirectory(incremental) )
        {
            if (!stoFileUtils::CreateDirectory(incremental, force))
                return false;
        }
        pLastSep = pNextSep;

    }
    while ( *pLastSep != '\0' );

    return true;
}


bool stoFileUtils::CreateDirectoryForFile( IN const char* path, bool bForce )
{
    // remove the file name from the path
    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    stoFileUtils::SplitPath(path, drive, dir, NULL, NULL);
    char newPath[_MAX_PATH];
    stoFileUtils::MergePath(newPath, drive, dir, NULL, NULL);
    return stoFileUtils::CreateDirectoryTree(newPath, bForce);
}


int32 stoFileUtils::GetFileModifiedTime(const char *path)
{
#if CORE_SYSTEM_PS3
    struct CellFsStat statResult;
    if (cellFsStat(path, &statResult)==CELL_FS_SUCCEEDED)
        return statResult.st_mtime;
#else
    struct stat buf;
    if ( stat(path, &buf) == 0 )
        return (int32)buf.st_mtime;
#endif
    return 0;
}

int32 stoFileUtils::GetFileSize( IN const char *fullPath )
{
#if CORE_SYSTEM_PS3
    struct CellFsStat statResult;
    if (cellFsStat(fullPath, &statResult)==CELL_FS_SUCCEEDED)
        return statResult.st_size;
#else
    struct stat buf;
    if ( stat(fullPath, &buf) == 0 )
        return buf.st_size;
#endif
    return -1;
}

/**
 *   Delete a file
 *
 *  @param fullPath - full path to the file to delete
 * 
 *  @return true if the file is deleted
 */
bool stoFileUtils::DeleteFile( IN const char *fullPath, IN bool force )
{
    if ( force )
    {
#if !CORE_SYSTEM_PS3
        if ( IsFile(fullPath) && IsReadOnly(fullPath) )
        {
            chmod(fullPath, _S_IREAD | _S_IWRITE);
        }
#endif
    }

    return unlink(fullPath) == 0;
}


bool stoFileUtils::MoveFile( IN const char *sourcePath, IN const char *destinationPath )
{
#ifdef WIN32
    return ::MoveFile(sourcePath, destinationPath) != 0;
#else
    return rename(sourcePath, destinationPath) != 0;
#endif
}

bool stoFileUtils::CopyFile( IN const char *sourcePath, IN const char *destinationPath )
{
#ifdef WIN32
    return ::CopyFile(sourcePath, destinationPath, true) != 0;
#else
    return rename(sourcePath, destinationPath) != 0;
#endif

}


struct PatternToken
{
    int             skipChars;
    bool            bFloating;
    char*           pLiteral;
    size_t          length;
    const char*     pMatch;
};


/**
 *  check if a pattern matches a file name.
 *
 *  @param fileName - the file name to compare
 *  @param pattern - the matching pattern
 * 
 *  @return true if the file name matches the pattern
 */
bool stoFileUtils::Match( IN const char *fileName, IN const char *pattern )
{
    if ( *pattern == '\0' )
    {
        return *fileName == '\0';
    }

    char newPattern[_MAX_PATH];
    strcpy(newPattern, pattern);

    // parse pattern into a vector of tokens
    vector<PatternToken> tokens;

    for ( const char *pChar = newPattern; *pChar != '\0'; /* no increment here */ )
    {
        PatternToken t;
        t.skipChars = 0;
        t.bFloating = false;

        while ( *pChar == '?' || *pChar == '*' )
        {
            if ( *pChar == '?' )
            {
                ++t.skipChars;
            }
            else
            {
                t.bFloating = true;
            }
            ++pChar;
        }
        t.pLiteral = const_cast<char *>(pChar);
        pChar = NextCharInString(pChar, "*?");
        t.length = pChar - t.pLiteral;
        tokens.push_back(t);
    }

    // null terminate all literals
    for ( vector<PatternToken>::iterator it = tokens.begin(); it != tokens.end(); ++it )
    {
        it->pLiteral[it->length] = '\0';
    }

    const char* pMatchChar = fileName;
    const char* pEndString = fileName + strlen(fileName);

    PatternToken* pFirstToken = &tokens.front();
    PatternToken* pLastToken = &tokens.back();
    PatternToken* pToken = pFirstToken;
    while ( true )
    {
        if ( pToken > pLastToken )
        {
            if ( *pMatchChar == '\0' )
                return true;
            else
                goto MatchFailed;
        }

        pMatchChar += pToken->skipChars;

        if ( pMatchChar > pEndString )
            return false;

        // trailing wildcard matches rest of string
        if ( pToken->length == 0 )
            return true;

RetryLiteralMatch:
        if ( pToken->bFloating )
            pToken->pMatch = FindString(pMatchChar, pToken->pLiteral);
        else
            pToken->pMatch = strncasecmp(pMatchChar, pToken->pLiteral, pToken->length) == 0 ? pMatchChar : NULL;

        if ( pToken->pMatch )
        {
            pMatchChar = pToken->pMatch + pToken->length;
            ++pToken;
            continue;
        }

MatchFailed:
        // look for a previous literal token that is preceded by a wild token ( index >= 1 )
        if ( pToken == pFirstToken )
            return false;
        --pToken;

        pMatchChar = pToken->pMatch + 1;
        goto RetryLiteralMatch;
    }
}


/*
    Generic wrapper for _splitpath behavior. Break a path name into components.

    Each argument is stored in a buffer; the manifest constants _MAX_DRIVE, 
    _MAX_DIR, _MAX_FNAME, and _MAX_EXT (defined in STDLIB.H) specify the 
    maximum size necessary for each buffer. The other arguments point to 
    buffers used to store the path elements. After a call to SplitPath is 
    executed, these arguments contain empty strings for components not found in 
    path. You can pass a NULL pointer to SplitPath for any component you don't 
    need.

    @param path     Full path
    @param drive    Optional drive letter, followed by a colon (:)
    @param dir      Optional directory path, including trailing slash. Forward 
                    slashes ( / ), backslashes ( \ ), or both may be used.
    @param fname    Base filename (no extension) 
    @param ext      Optional filename extension, including leading period (.) 

    @return none.
*/
void stoFileUtils::SplitPath( IN const char *path, OUT char *drive, 
                             OUT char *dir, OUT char *fname, OUT char *ext )
{
    const char* pDirStart = path;

#if CORE_SYSTEM_WINAPI
    if ( isalpha(path[0]) && path[1] == ':' )
    {
        pDirStart += 2;
    }
#endif

    if ( drive )
    {
        memcpy(drive, path, pDirStart - path);
        drive[pDirStart - path] = '\0';
    }

    // find the right-most directory separator and point one past it
    const char *pFnameStart = pDirStart;
    for ( const char* pChar = pDirStart; *pChar != '\0'; ++pChar )
    {
        for ( const char* pTestChar = gDirSeps; *pTestChar != '\0'; ++pTestChar )
        {
            if ( *pChar == *pTestChar )
            {
                pFnameStart = pChar + 1;
                break;
            }
        }
    }

    if ( dir )
    {
        if ( pFnameStart >  pDirStart )
        {
            memcpy(dir, pDirStart, pFnameStart - pDirStart);
        }
        dir[pFnameStart - pDirStart] = '\0';
    }

    // find "." divider
    const char* pDot = NextCharInString(pFnameStart, ".");

    if ( fname )
    {
        memcpy(fname, pFnameStart, pDot - pFnameStart);
        fname[pDot - pFnameStart] = '\0';
    }

    if ( ext )
    {
        strcpy(ext, pDot);
    }
}


/*
    Generic wrapper for _makepath behavior. Create a path name from components.

    The path argument must point to an empty buffer large enough to hold the 
    complete path. Although there are no size limits on any of the fields that 
    constitute path, the composite path must be no larger than the _MAX_PATH 
    constant, defined in STDLIB.H. _MAX_PATH may be larger than the current 
    operating-system version will handle.

    @param path     Full path buffer
    @param drive    Contains a letter (A, B, and so on) corresponding to the 
                    desired drive and an optional trailing colon. MergePath
                    inserts the colon automatically in the composite path if it 
                    is missing. If drive is a null character or an empty string,
                    no drive letter and colon appear in the composite path 
                    string. 
    @param dir      Contains the path of directories, not including the drive 
                    designator or the actual filename. The trailing slash is 
                    optional, and either a forward slash (/) or a backslash (\) 
                    or both may be used in a single dir argument. If a trailing 
                    slash (/ or \) is not specified, it is inserted 
                    automatically. If dir is a null character or an empty 
                    string, no slash is inserted in the composite path string. 
    @param fname    Contains the base filename without any extensions. If fname 
                    is NULL or points to an empty string, no filename is 
                    inserted in the composite path string. 
    @param ext      Contains the actual filename extension, with or without a 
                    leading period (.). MergePath inserts the period 
                    automatically if it does not appear in ext. If ext is a null 
                    character or an empty string, no period is inserted in the 
                    composite path string. 
    @return none.

    The path argument must point to an empty buffer large enough to hold the 
    complete path. Although there are no size limits on any of the fields that 
    constitute path, the composite path must be no larger than the _MAX_PATH 
    constant, defined in STDLIB.H. _MAX_PATH may be larger than the current 
    operating-system version will handle.
*/
void stoFileUtils::MergePath( OUT char *path, IN const char *drive, 
                             IN const char *dir, IN const char *fname, 
                             IN const char *ext )
{
    errAssert(path != NULL);
    path[0] = '\0';

    if ( drive )
        strcat(path, drive);

    bool bSlashNeeded = false;
    if ( dir )
    {
        strcat(path, dir);

        // find the last char in the string to determine whether we need to add a "/" before the fname
        if ( dir[0] != '\0' )
        {
            char lastChar = dir[strlen(dir) - 1];

            if ( strchr(gDirSeps, lastChar) == 0 )
                bSlashNeeded = true;
        }
    }


    if ( (fname || ext) && bSlashNeeded )
        strcat(path, "/");

    if ( fname )
    {
        strcat(path, fname);
    }

    if ( ext )
    {
        if ( ext[0] != '.' )
            strcat(path, ".");
        strcat(path, ext);
    }
}


/*
 *  add a file extension to a filename if the filename
 *  doesn't have an extension yet. this doesn't check the size of
 * the output buffer
 *
 *  @param originalName - the file name to compare
 *  @param extension - the matching pattern
 *  @param buffer - output buffer to write the new filename into
 * 
 *  @return none.
 */
void stoFileUtils::AddDefaultExtension( IN const char *originalName, IN const char *extension, 
                                       OUT char *outName )
{
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
    stoFileUtils::SplitPath(originalName, drive, dir, fname, ext);

    if ( ext[0] == '\0' )
        strcpy(ext, extension);

    stoFileUtils::MergePath(outName, drive, dir, fname, ext);
}


/*
 * build a list of the files within a directory and it's
 * sub-directories.
 *
 *  @param dirName - the starting directory
 *  @param output - the vector to put the file names in
 * 
 *  @return errResult.
 */
errResult stoFileUtils::BuildRecursiveFileList(const char *dirName, vector<string> &output)
{
    if (dirName == NULL)
        return ER_Failure;

    if (!stoFileUtils::IsDirectory(dirName) )
        return ER_Failure;

    vector<string> files;
    stoFileUtils::ListDirectory(dirName, files);
    for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
    {
        string fileName = *it;
        string path = string(dirName) + "/" + fileName;
        if (stoFileUtils::IsDirectory(path.c_str()) )
        {
            BuildRecursiveFileList(path.c_str(), output);
        } else {
            output.push_back(path);
        }
    }
    return ER_Success;
}


errResult stoFileUtils::BuildRecursiveFileListWithExclude(const char *dirName, vector<string> &output, vector<string> &excludeDirs)
{
    if (dirName == NULL)
        return ER_Failure;

    if (!stoFileUtils::IsDirectory(dirName) )
        return ER_Failure;

    vector<string> files;
    stoFileUtils::ListDirectory(dirName, files);
    for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
    {
        string fileName = *it;
        string path;
        if (strlen(dirName) == 0)
        {
            path = fileName;
        } else {
            path = string(dirName) + "/" + fileName;
        }

        if (stoFileUtils::IsDirectory(path.c_str()) )
        {
            bool ignore = false;
            // check for excluded dirs
            for (vector<string>::iterator it2 = excludeDirs.begin(); it2 != excludeDirs.end(); it2++)
            {
                string &excludeDir = *it2;
                string fullExcludeDir = string(dirName) + "/" + excludeDir;
                if (strcasecmp(fullExcludeDir.c_str(), path.c_str()) == 0)
                {
                    ignore = true;
                    break;
                }
            }
            if (!ignore)
                BuildRecursiveFileListWithExclude(path.c_str(), output, excludeDirs);
        } else {
            output.push_back(path);
        }
    }
    return ER_Success;
}


/** 
 * create a temporary file
 */
errResult stoFileUtils::GetTempFileName(const char *directory, const char *prefix, string &name)
{
#if CORE_SYSTEM_WINAPI
  char *newName = _tempnam(directory, prefix);
  if (newName)
  {
    name = newName;

    // _tempnam() pointers must be free()'d
    free( newName );

    return ER_Success;
  }
  else
  {
    name = "";
    return ER_Failure;
  }

#else

  UNREF(directory);
  char buf[512];
  sprintf(buf, "%sXXXXXX", prefix);
  
  int fd = mkstemp(buf);
  if (fd  == -1)
  {
    name = "";
    return ER_Failure;
  }
  else
  {
    name = buf;
    // this actually creates the file. 
    close(fd);
    unlink(buf);
    return ER_Success;
  }
#endif
}

#if CORE_SYSTEM_WINAPI
// strips off useless and dangerous quotation marks
static string stoStripQuotes( const char* inString )
{
    string output;
    for ( const char* current = inString; *current; ++current )
    {
        if ( *current != '\"' )
        {
            output += *current;
        }
    }

    return output;
}
#endif

/**
 * gets the directory that the current program was executed from.
 */
string stoFileUtils::GetExecutableDirectory()
{
#if CORE_SYSTEM_WINAPI
    char path[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];
    stoFileUtils::SplitPath( stoStripQuotes( ::GetCommandLine() ).c_str(),
        drive, dir, NULL, NULL);
    stoFileUtils::MergePath(path, drive, dir, NULL, NULL);
    return string(path);
#else
    ERR_UNIMPLEMENTED();
    return string("");
#endif
}


#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/core/corStlAlgorithm.h"
#include <fcntl.h>

// utility function
void ConvertPathSlashes( char *p )
{
#if CORE_SYSTEM_XENON
    ASSERTV(0,("Not on 360"));
#endif

    while ( true )
    {
        p = strchr(p, '\\');
        if ( p == NULL )
            break;
        *p = '/';
        ++p;
    }
}

class stoFileUtilsTests : public tstUnit
{
public:
	stoFileUtilsTests()
	{
	}

	virtual void Register()
	{
		SetName("stoFileUtils");

		AddTestCase("SplitPath()", &stoFileUtilsTests::TestSplitPath);
		AddTestCase("MergePath()", &stoFileUtilsTests::TestMergePath);
		AddTestCase("AddDefaultExtension()", &stoFileUtilsTests::TestAddDefaultExtension);
		AddTestCase("GetExecutableDirectory()", &stoFileUtilsTests::TestGetExecutableDirectory);
		AddTestCase("CreateDirectory()", &stoFileUtilsTests::TestCreateDirectory);
		AddTestCase("FileExists()", &stoFileUtilsTests::TestFileExists);
		AddTestCase("IsReadOnly()", &stoFileUtilsTests::TestIsReadOnly);
		AddTestCase("IsFile()", &stoFileUtilsTests::TestIsFile);
		AddTestCase("IsDirectory()", &stoFileUtilsTests::TestIsDirectory);
		AddTestCase("DeleteDirectory()", &stoFileUtilsTests::TestDeleteDirectory);
		AddTestCase("ListDirectory()", &stoFileUtilsTests::TestListDirectory);
		AddTestCase("CreateDirectoryTree()", &stoFileUtilsTests::TestCreateDirectoryTree);
		AddTestCase("CreateDirectoryForFile()", &stoFileUtilsTests::TestCreateDirectoryForFile);
		AddTestCase("Match()", &stoFileUtilsTests::TestMatch);
	};

	virtual void UnitTestSetup()
	{
		// set up code
	}

	virtual void UnitTestTearDown()
	{
		// tear down code
	}

    void TestSplitPath() const
    {
        char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
        stoFileUtils::SplitPath("file.txt", drive, dir, fname, ext);
        TESTASSERT(strcmp(drive, "") == 0);
        TESTASSERT(strcmp(dir, "") == 0);
        TESTASSERT(strcmp(fname, "file") == 0);
        TESTASSERT(strcmp(ext, ".txt") == 0);

        stoFileUtils::SplitPath("c:/dir/file.txt", drive, dir, fname, ext);
#if CORE_SYSTEM_WINAPI
        TESTASSERT(strcmp(drive, "c:") == 0);
        TESTASSERT(strcmp(dir, "/dir/") == 0);
#elif CORE_SYSTEM_LINUX
        TESTASSERT(strcmp(drive,"") == 0);
        TESTASSERT(strcmp(dir, "c:/dir/") == 0);
#endif
        TESTASSERT(strcmp(fname, "file") == 0);
        TESTASSERT(strcmp(ext, ".txt") == 0);

        stoFileUtils::SplitPath("c:/dir1/dir2/file.txt", drive, dir, fname, ext);
#if CORE_SYSTEM_WINAPI
        TESTASSERT(strcmp(drive, "c:") == 0);
        TESTASSERT(strcmp(dir, "/dir1/dir2/") == 0);
#elif CORE_SYSTEM_LINUX
        TESTASSERT(strcmp(drive, "") == 0);
        TESTASSERT(strcmp(dir, "c:/dir1/dir2/") == 0);
#endif
        TESTASSERT(strcmp(fname, "file") == 0);
        TESTASSERT(strcmp(ext, ".txt") == 0);

        stoFileUtils::SplitPath("c:/Program Files/XYZ Corporation/Really long path/Big File Name.big extension", drive, dir, fname, ext);
#if CORE_SYSTEM_WINAPI
        TESTASSERT(strcmp(drive, "c:") == 0);
        TESTASSERT(strcmp(dir, "/Program Files/XYZ Corporation/Really long path/") == 0);
#elif CORE_SYSTEM_LINUX
        TESTASSERT(strcmp(drive, "") == 0);
        TESTASSERT(strcmp(dir, "c:/Program Files/XYZ Corporation/Really long path/") == 0);
#endif
        TESTASSERT(strcmp(fname, "Big File Name") == 0);
        TESTASSERT(strcmp(ext, ".big extension") == 0);

        stoFileUtils::SplitPath("//machine name/directory.a/directory.b/../filename.extension", drive, dir, fname, ext);
        TESTASSERT(strcmp(drive, "") == 0);
        TESTASSERT(strcmp(dir, "//machine name/directory.a/directory.b/../") == 0);
        TESTASSERT(strcmp(fname, "filename") == 0);
        TESTASSERT(strcmp(ext, ".extension") == 0);
    }

    void TestMergePath() const
    {
        char path[_MAX_PATH];

        stoFileUtils::MergePath(path, NULL, NULL, NULL, NULL);
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "") == 0);

        stoFileUtils::MergePath(path, "c:", "/dir1/dir2/", "filename", ".ext");
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "c:/dir1/dir2/filename.ext") == 0);

        stoFileUtils::MergePath(path, "c:", "/dir1/dir2", "filename", "ext");   
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "c:/dir1/dir2/filename.ext") == 0);

        stoFileUtils::MergePath(path, "c:", NULL, "filename", "ext");   
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "c:filename.ext") == 0);

        stoFileUtils::MergePath(path, "c:", "/dir1/dir2", NULL, "ext");   
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "c:/dir1/dir2/.ext") == 0);

        stoFileUtils::MergePath(path, "c:", "/dir1/dir2", "filename", NULL);   
        ConvertPathSlashes(path);
        TESTASSERT(strcmp(path, "c:/dir1/dir2/filename") == 0);
    }

    void TestAddDefaultExtension() const
    {
        char path[_MAX_PATH];

        stoFileUtils::AddDefaultExtension("filename.txt", "exe", path);
        TESTASSERT(strcmp(path, "filename.txt") == 0);

        stoFileUtils::AddDefaultExtension("filename", "exe", path);
        TESTASSERT(strcmp(path, "filename.exe") == 0);

        stoFileUtils::AddDefaultExtension("filename", ".exe", path);
        TESTASSERT(strcmp(path, "filename.exe") == 0);

        stoFileUtils::AddDefaultExtension("c:/dir.ext/dir.ext/filename", ".exe", path);
        TESTASSERT(strcmp(path, "c:/dir.ext/dir.ext/filename.exe") == 0);

        stoFileUtils::AddDefaultExtension("c:/dir.ext/dir.ext/filename.txt", ".exe", path);
        TESTASSERT(strcmp(path, "c:/dir.ext/dir.ext/filename.txt") == 0);
    }

    void TestGetExecutableDirectory() const
    {
#if CORE_SYSTEM_WINAPI
        // sucks that we can only test this on windows...
        char path[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];
        strcpy(path, stoStripQuotes( ::GetCommandLine() ).c_str() );
        stoFileUtils::SplitPath(path, drive, dir, NULL, NULL);
        stoFileUtils::MergePath(path, drive, dir, NULL, NULL);
        TESTASSERT(strcmp(path, stoFileUtils::GetExecutableDirectory().c_str()) == 0);
#endif
    }

    void TestFileExists() const
    {
        string pathStr;
        if (ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_FileExists", pathStr)))
	{
	  const char *path = pathStr.c_str();
          TESTASSERT(stoFileUtils::FileExists(path) == false);

          int hFile = open(path, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE | _S_IREAD);
          close(hFile);

          TESTASSERT(stoFileUtils::FileExists(path) == true);

          unlink(path);
          TESTASSERT(stoFileUtils::FileExists(path) == false);

	  stoFileUtils::CreateDirectory(path);
          TESTASSERT(stoFileUtils::FileExists(path) == true);

          rmdir(path);
          TESTASSERT(stoFileUtils::FileExists(path) == false);
      }
    }


    void TestIsReadOnly() const
    {
// Can't test RO currently CellOS 0.5.13-
#if !CORE_SYSTEM_PS3 
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_IsReadOnly", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        TESTASSERT(stoFileUtils::IsReadOnly(path) == false);

        int hFile = open(path, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE | _S_IREAD);
        close(hFile);

        TESTASSERT(stoFileUtils::IsReadOnly(path) == false);

        chmod(path, _S_IREAD);
        TESTASSERT(stoFileUtils::IsReadOnly(path) == true);

        chmod(path, _S_IWRITE);
        TESTASSERT(stoFileUtils::IsReadOnly(path) == false);

        unlink(path);
#endif
    }

    void TestIsFile() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_IsFile", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        TESTASSERT(stoFileUtils::FileExists(path) == false);

        int hFile = open(path, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE | _S_IREAD);
        close(hFile);
        TESTASSERT(stoFileUtils::IsFile(path) == true);
        unlink(path);

        stoFileUtils::CreateDirectory(path);
        TESTASSERT(stoFileUtils::IsFile(path) == false);

        rmdir(path);

    }

    void TestIsDirectory() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_IsDirectory", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        TESTASSERT(stoFileUtils::IsDirectory(path) == false);

        int hFile = open(path, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE | _S_IREAD);
        close(hFile);
        TESTASSERT(stoFileUtils::IsDirectory(path) == false);
        unlink(path);

        stoFileUtils::CreateDirectory(path);
        TESTASSERT(stoFileUtils::IsDirectory(path) == true);

        rmdir(path);

    }

    void TestCreateDirectory() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_CreateDirectory", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        TESTASSERT(stoFileUtils::IsDirectory(path) == false);

        stoFileUtils::CreateDirectory(path);
        TESTASSERT(stoFileUtils::IsDirectory(path) == true);

        rmdir(path);

    }

    void TestDeleteDirectory() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_DeleteDirectory", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        stoFileUtils::CreateDirectory(path);
        TESTASSERT(stoFileUtils::IsDirectory(path) == true);

        // create a bunch of dummy files in the directory
        for ( int i = 0; i < 16; ++i )
        {
            char filename[_MAX_FNAME], path2[_MAX_PATH];
            sprintf(filename, "dummy%04i", i);
            stoFileUtils::MergePath(path2, NULL, path, filename, NULL);

            int flags = 0;
            if ( i & 1 )
                flags |= _S_IREAD;
            if ( i & 2 )
                flags |= _S_IWRITE;
            close(open(path2, O_WRONLY | O_CREAT | O_TRUNC, flags));
        }

        stoFileUtils::DeleteDirectory(path);
        TESTASSERT(stoFileUtils::IsDirectory(path) == false);
        TESTASSERT(stoFileUtils::IsFile(path) == false);
        TESTASSERT(stoFileUtils::FileExists(path) == false);

    }

    void TestListDirectory() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_ListDirectory", pathStr)))
	  return;

	const char *dir = pathStr.c_str();

        stoFileUtils::CreateDirectory(dir);

        char path2[_MAX_PATH];

        stoFileUtils::MergePath(path2, NULL, dir, "fileA", NULL);
        close(open(path2, O_WRONLY | O_CREAT | O_TRUNC, _S_IREAD |_S_IWRITE));

        stoFileUtils::MergePath(path2, NULL, dir, "fileB", NULL);
        close(open(path2, O_WRONLY | O_CREAT | O_TRUNC, _S_IREAD |_S_IWRITE));

        stoFileUtils::MergePath(path2, NULL, dir, "fileC", NULL);
        close(open(path2, O_WRONLY | O_CREAT | O_TRUNC, _S_IREAD |_S_IWRITE));

        vector<string> results;
        stoFileUtils::ListDirectory(dir, results);

        sort(results.begin(), results.end());

        TESTASSERT(results.size() == 3);
        TESTASSERT(strcmp(results.at(0).c_str(), "fileA") == 0);
        TESTASSERT(strcmp(results.at(1).c_str(), "fileB") == 0);
        TESTASSERT(strcmp(results.at(2).c_str(), "fileC") == 0);

        stoFileUtils::DeleteDirectory(dir);
    }

    void TestCreateDirectoryTree() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_CreateDirectoryTree", pathStr)))
	  return;

	const char *path = pathStr.c_str();

        char dir1[_MAX_PATH];
        strcpy(dir1, path);
        strcat(dir1, "/a");
        char dir2[_MAX_PATH];
        strcpy(dir2, dir1);
        strcat(dir2, "/b");
        char dir3[_MAX_PATH];
        strcpy(dir3, dir2);
        strcat(dir3, "/c");
        char dir4[_MAX_PATH];
        strcpy(dir4, dir3);
        strcat(dir4, "/d");
        char dir5[_MAX_PATH];
        strcpy(dir5, dir4);
        strcat(dir5, "/e");

        TESTASSERT(stoFileUtils::IsDirectory(path) == false);

        TESTASSERT(stoFileUtils::CreateDirectoryTree(dir5));

        TESTASSERT(stoFileUtils::IsDirectory(path) == true);
        TESTASSERT(stoFileUtils::IsDirectory(dir1) == true);
        TESTASSERT(stoFileUtils::IsDirectory(dir2) == true);
        TESTASSERT(stoFileUtils::IsDirectory(dir3) == true);
        TESTASSERT(stoFileUtils::IsDirectory(dir4) == true);
        TESTASSERT(stoFileUtils::IsDirectory(dir5) == true);

        stoFileUtils::DeleteDirectory(path);
    }

    static void BuildPath( char* outPath, const char* inBasePath,
        const char* inAdditionalPathInfo )
    {
        strcpy( outPath, inBasePath );
        strcat( outPath, inAdditionalPathInfo );
    }

    void TestCreateDirectoryForFile() const
    {
        string pathStr;
        if (!ISOK(stoFileUtils::GetTempFileName(NULL, "stoFileUtilsTest_CreateDirectoryForFile", pathStr)))
	  return;

	const char *basepath = pathStr.c_str();

        char path[_MAX_PATH];

        BuildPath( path, basepath, "/a/b/c/d/file.txt" );
        TESTASSERT(stoFileUtils::CreateDirectoryForFile(path));

        BuildPath( path, basepath, "/a" );
        TESTASSERT(stoFileUtils::IsDirectory( path ) == true);

        BuildPath( path, basepath, "/a/b" );
        TESTASSERT(stoFileUtils::IsDirectory( path ) == true);

        BuildPath( path, basepath, "/a/b/c" );
        TESTASSERT(stoFileUtils::IsDirectory( path ) == true);

        BuildPath( path, basepath, "/a/b/c/d" );
        TESTASSERT(stoFileUtils::IsDirectory( path ) == true);

        BuildPath( path, basepath, "/a/b/c/d/file.txt" );
        TESTASSERT( stoFileUtils::FileExists( path ) == false );

        stoFileUtils::DeleteDirectory(basepath);
    }

    void TestMatch() const
    {
        TESTASSERT(stoFileUtils::Match("", "*") == true);
        TESTASSERT(stoFileUtils::Match("", "?") == false);
        TESTASSERT(stoFileUtils::Match("", "") == true);
        TESTASSERT(stoFileUtils::Match("something", "") == false);
        TESTASSERT(stoFileUtils::Match("fileA", "*") == true);
        TESTASSERT(stoFileUtils::Match("fileB.extB", "*") == true);
        TESTASSERT(stoFileUtils::Match("fileC.extC", "*.*") == true);
        TESTASSERT(stoFileUtils::Match("fileD.extD", "*.") == false);
        TESTASSERT(stoFileUtils::Match("fileE.extE", "*.txt") == false);
        TESTASSERT(stoFileUtils::Match("fileF.extF", "*.extF") == true);
        TESTASSERT(stoFileUtils::Match("fileG.extG", "fileG.*") == true);
        TESTASSERT(stoFileUtils::Match("fileH.extH", "fileH.extH") == true);
        TESTASSERT(stoFileUtils::Match("fileI.extI", "f*.e*") == true);
        TESTASSERT(stoFileUtils::Match("fileJ.extJ", "x*.e*") == false);
        TESTASSERT(stoFileUtils::Match("fileK.extK", "f*.x*") == false);
        TESTASSERT(stoFileUtils::Match("fileL.extL", "*L.*L") == true);
        TESTASSERT(stoFileUtils::Match("fileM.extM", "*_.*M") == false);
        TESTASSERT(stoFileUtils::Match("fileN.extN", "*N.*_") == false);
        TESTASSERT(stoFileUtils::Match("fileO.extO", "f*O.e*O") == true);
        TESTASSERT(stoFileUtils::Match("fileP.extP", "f*_.e*P") == false);
        TESTASSERT(stoFileUtils::Match("fileQ.extQ", "f*Q.e*_") == false);
        TESTASSERT(stoFileUtils::Match("fileR.extR", "f*l*R.*x*R") == true);
        TESTASSERT(stoFileUtils::Match("fileS.extS", "f*l*_.*x*_") == false);
        TESTASSERT(stoFileUtils::Match("fileT.extT", "?????.????") == true);
        TESTASSERT(stoFileUtils::Match("fileU.extU", "f?l?U.?x?U") == true);
        TESTASSERT(stoFileUtils::Match("fileV.extV", "f?l?_.?x?_") == false);
        TESTASSERT(stoFileUtils::Match("fileW.extW", "fileW?.extW?") == false);
        TESTASSERT(stoFileUtils::Match("fileX.extX", "*fileX*.*extX*") == true);
        TESTASSERT(stoFileUtils::Match("fileY.extY", "f*e?.*?") == true);
        TESTASSERT(stoFileUtils::Match("fileZ.extZ", "?il*.?x*)") == false);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*be*the*question*") == true);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*be? *not*") == false);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*be*t*t*the*") == true);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*,*t*t*t*is*") == false);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*n?t*t?e*") == true);
        TESTASSERT(stoFileUtils::Match("To be or not to be, that is the question.", "*t??t*,*") == false);
        TESTASSERT(stoFileUtils::Match("All cows eat grass", "*c?w? ?a? ?r*") == true);
        TESTASSERT(stoFileUtils::Match("All cows eat grass", "*c?w? ?a? ?a*") == false);
        TESTASSERT(stoFileUtils::Match("All cows eat grass", "cows*") == false);
    }
};

EXPORTUNITTESTOBJECT(stoFileUtilsTests);

#endif // BUILD_UNIT_TESTS
