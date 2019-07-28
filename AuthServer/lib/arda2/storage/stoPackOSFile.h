/*****************************************************************************
    created:      2003/1/13
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Sean Riley
    
    purpose:      stoPackOSFile is a pack file on disk used by the PackManager.
*****************************************************************************/

#ifndef INCLUDED_CORE_PACKOSFILE_H
#define INCLUDED_CORE_PACKOSFILE_H

#include "arda2/util/utlStringTable.h"
#include "arda2/core/corStlVector.h"
#include "arda2/core/corStlMap.h"
#include "arda2/core/corStlHashmap.h"
#include "arda2/storage/stoFile.h"
#include "arda2/storage/stoFileOSFile.h"

class stoPackOSFile;

/// Internal in-memory structure for file entry.
struct FileEntry
{
    int32 dataOffset;           ///< Offset into data segment of file.
    int32 dataSize;             ///< Size of data of file
    int32 realSize;             ///< the uncompressed size of the data
    int32 modifiedTime;         ///< last time the file was modified
    utlStringId name;                   ///< my filename
    int16       scheme;                 ///< type of compression..
    stoPackOSFile* packFile;            ///< the packFile I came from (not written to pack file)
};

typedef std::vector<FileEntry> FileEntryVec;
typedef HashMap< utlStringId, FileEntryVec, utlStringIdHashTraits > Directory;  ///< Type for index file.

inline FileEntry* GetEntryFromDirectory(const utlStringId& id, Directory& directory)
{
    Directory::iterator dirIt = directory.find(id);
    if (dirIt == directory.end())
    {
        return NULL;
    }
    if ((*dirIt).second.size() == 0)
    {
        return NULL;
    }
    return (&((*dirIt).second.back()));
}

inline const FileEntry* GetEntryFromDirectory(const utlStringId& id, const Directory& directory)
{
    Directory::const_iterator dirIt = directory.find(id);
    if (dirIt == directory.end())
    {
        return NULL;
    }
    if ((*dirIt).second.size() == 0)
    {
        return NULL;
    }
    return (&((*dirIt).second.back()));
}


bool FileEntryCmp(const FileEntry &e1, const FileEntry &e2);
bool FileEntryNameCmp(const FileEntry &e1, const char *fileName);
bool FileEntryNameIdCmp(const FileEntry &e1, const utlStringId& fileName);
void GetFileName(const char *path, char *fileName, const char *baseDir = NULL);

// Useful classes for parsing the global dictionary to strip down to the needed
// data for our index
struct stoPackOSFileEntrySet
{
    int32               m_stringOffset;
    std::vector<FileEntry*>  m_entries;
};

// Comparison functor
struct stoPackOSFileStringIdValueCmp
{
    inline bool operator()(const utlStringId& s1, const utlStringId& s2) const
    {
        return _stricmp(g_stringTable.GetString(s1), g_stringTable.GetString(s2)) < 0;
    }
};

typedef std::map< utlStringId, stoPackOSFileEntrySet, stoPackOSFileStringIdValueCmp> SortedPackFileEntries;


/**
 *  Class for a pack file that is managed by the packManager.
 *  The directory of files is managed in the packManager, not here.
 
The structure of the pack file is:

|------ file data ------| |--- string table ---| |--- index ---| |- index size -|

NOTE: this class isnt intended for use outside of the stoPackManager.

Each stoPackOSFile keeps a list of empty slots within it for re-using
deleted space. These entries have string names of stoPackOSFile::labelDeleted 
and are at the end of the index on disk.

**/

class stoPackOSFile
{
public:
    stoPackOSFile(const bool readonly);
    ~stoPackOSFile();

    errResult Init(const char *fileName, Directory &directory);
    errResult Add(const char *fileName);
    errResult Flush(Directory &directory);
    void Close();

    errResult Write(const char *fileName, void *buffer, int32 realSize, FileEntry &entry);
    errResult Delete(FileEntry &entry);
    const char *GetPackName() const { return m_fileName; }
    const int32 GetEmptySlots() const { return (const int32)(m_deleted.size()); }
    int32 ListDeleted(); // returns total size of empty space

    FileEntryVec& GetDeleted() { return m_deleted; }

    // check if the os file of this pack file is the specified file
    bool IsFile(stoFileOSFile *file) const;

    bool CanWrite() const
    {
        return m_packFile.CanWrite();
    }

    stoFileOSFile& GetFileOSFile(){ return m_packFile; }

private:
    errResult InitIndex(Directory &directory, const int indexPos, const int32 totalSize);
    errResult InitStringTable(HashMap<int, utlStringId>& filenameLookup, const int32 numEntries);

    stoFileOSFile m_packFile;
    const char*   m_fileName;
    int32         m_dataSize;
    FileEntryVec  m_deleted;
    const bool    m_readonly;

    static int32 BlockSize(int32 size);

    static const utlStringId labelDeleted; // ID_NONE
    static const int32  BLOCK_SIZE;

    // no copy or assignment
    stoPackOSFile(const stoPackOSFile& rhs);
    const stoPackOSFile& operator=(const stoPackOSFile& rhs);

};

#endif // !INCLUDED_CORE_PACKOSFILE_H

