/**
 *  It implements stoPackManager class and File class.
 *
 *  Basically stoPackManager combines many small files into one big file to increase
 *  performance, to hide details from users, to save hard disk
 *  space.
 *
 *  @author Jake Song
 *
 *  @date 7/10/2001
 *
 *  @file
**/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFileView.h"
#include "arda2/storage/stoChunk.h"
#include "arda2/storage/stoPackOSFile.h"
#include "arda2/storage/stoFileUtils.h"

#include "arda2/core/corStlAlgorithm.h"

using namespace std;

const utlStringId stoPackOSFile::labelDeleted;
const int32 stoPackOSFile::BLOCK_SIZE = 1024;

/**
 *  Constructor.
 *  Call Init() before using any other member functions.
**/
stoPackOSFile::stoPackOSFile(const bool readonly):
    m_fileName(NULL),
    m_dataSize(0),
    m_readonly(readonly)
{
}

/**
 *  Destructor.
 *  Call Flush() before destructing if you added or deleted files.
**/
stoPackOSFile::~stoPackOSFile()
{
    m_packFile.Close();
    if (m_fileName)
        free((void*)m_fileName);
}

/**
 *  the Init has been changed from the old FileMan. The new 
 *  Init opens the pack file, reads the index size from the 
 *  end of the file, then loads the index into memory. There
 *  is no longer an index file
 *
 *  Once this method completes, the index data on disk can be
 *  written over as it is now in memory.
 *
 *  @param fileName file name or path of GLM file without extension.
 *
 *  @return ER_Success if success, ER_Failure if fail
**/
errResult stoPackOSFile::Init(const char *fileName, Directory &directory)
{
  m_fileName = strdup(fileName);
  char glmPath[_MAX_PATH];
  stoFileUtils::AddDefaultExtension(fileName, "glm", glmPath);

  // use stoFileOSFile instead of stdio
  stoFileOSFile::AccessMode mode = stoFileOSFile::kAccessReadWrite;
  if (m_readonly)
      mode = stoFileOSFile::kAccessRead;

  if (!stoFileUtils::IsFile(glmPath))
  {
      // doesnt exist.. create new file.
      if (m_packFile.Open(glmPath, stoFileOSFile::kAccessCreate) != ER_Success)
        return ER_Failure;
  }
  else
  {
      // open existing file.
      if (m_packFile.Open(glmPath, mode) != ER_Success)
        return ER_Failure;
  }

  // find size of file
  int32 totalSize = m_packFile.GetSize();
  if (totalSize == 0)
      return ER_Success;

  if (totalSize == -1)
      return ER_Failure;

  // read in index position
  int32 indexPos=-1;
  if (m_packFile.Seek(totalSize-sizeof(int32), stoFileOSFile::kSeekBegin) != ER_Success)
      return ER_Failure;
  if (m_packFile.Read(&indexPos, sizeof(int32) ) != ER_Success)
      return ER_Failure;

  return InitIndex(directory, indexPos, totalSize);
}

/**
 *  InitIndex is called by init to load the pack file's index
 *  into memory. It does not read in any actual data from packed files.
 *
 *  @param directory - the in-memory directory of file entries
 *  @param indexPos - the position of the index in the file
 *  @param totalSize - the total size of the pack file
 *
 *  @return ER_Success if success, ER_Failure if fail
**/
errResult stoPackOSFile::InitIndex(Directory &directory, const int indexPos, const int32 totalSize)
{
  // create chunk
  int32 indexSize = (totalSize - indexPos) - sizeof(int32) ;
  stoViewFile view(m_packFile, indexPos, indexSize );
  m_packFile.Seek(indexPos, stoFileOSFile::kSeekBegin);
  stoChunkFileReader reader(view);

  // read in header information
  int32 stringSize;
  int32 numEntries;
  reader.Get(m_dataSize);
  reader.Get(stringSize);
  reader.Get(numEntries);

  //allocate temp stringtable
    HashMap<int, utlStringId> filenameLookup;

    errResult result = InitStringTable(filenameLookup, stringSize);
    if ( ISOK(result) )
    {
        // load each of the file entries
        FileEntry newEntry;
        newEntry.packFile = this;
        int32 stringOffset;

        for (int i=0; i != numEntries; ++i)
        {
            // load data
            reader.Get(newEntry.dataOffset);
            reader.Get(newEntry.dataSize);
            reader.Get(newEntry.realSize);
            reader.Get(stringOffset);
            reader.Get(newEntry.scheme);
            reader.Get(newEntry.modifiedTime);

            if (stringOffset == -1)
            {
                newEntry.name = stoPackOSFile::labelDeleted; 
                m_deleted.push_back(newEntry);
                continue;
            } 
            else 
            {
                // find string in string table
                HashMap<int, utlStringId>::iterator fileId = filenameLookup.find(stringOffset);
                errAssert(fileId != filenameLookup.end());

                newEntry.name = (*fileId).second;
            }

            // if the entry already exists in the directory, insert this one before the old one
            // one at the end of the contained vector of duplicates
            FileEntryVec& entries = directory[newEntry.name];
            entries.push_back(newEntry);
        }
  }

  //Rewind to beginning of data segment
  m_packFile.Seek(0, stoFileOSFile::kSeekBegin);
    
  return result;
}

/**
 *  InitStringTable - loads the string table into memory from
 * the pack file. 
 *
 *  @param tempStringTable - space to read the string table into
 *  @param stringSize - size of string table
 *
 *  @return ER_Success if success, ER_Failure if fail
**/
errResult stoPackOSFile::InitStringTable(HashMap<int, utlStringId>& filenameLookup, const int32 stringSize)
{
    if (m_packFile.Seek(m_dataSize, stoFileOSFile::kSeekBegin) != ER_Success)
        return ER_Failure;

    char* tempStringTable = new char[stringSize];

    if (m_packFile.Read(tempStringTable, stringSize) != ER_Success)
    {
        delete[] tempStringTable;
        return ER_Failure;
    }
    if (m_packFile.Tell() != m_dataSize + stringSize)
    {
        delete[] tempStringTable;
        return ER_Failure;
    }
    // Add all of the strings in the temporary string table to the global string
    // table, and build up our lookup hash map so that we can later associate
    // all file entries with the correct string Id
    int32 stringOffset = 0;

    while (stringOffset < stringSize)
    {
        utlStringId fileStringId = g_stringTable.AddString(tempStringTable + stringOffset);
        filenameLookup[stringOffset] = fileStringId;
        stringOffset += (int32)(strlen(tempStringTable + stringOffset)) + 1;
    }

    delete[] tempStringTable;
    return ER_Success;
}

/**
 *  Write out index to the end of the file.
 *
 *  @return true if success, ER_Failure if fail.
**/
errResult stoPackOSFile::Flush(Directory &directory)
{
    // Build us up a table of pointers to all file entries associated
    // with this pack file, sorted by the VALUE of their string id.  Also use
    // the map write out the string table while caching the string offsets for
    // use when writing out the file entries
    SortedPackFileEntries sortedEntries;

    if (m_packFile.Seek(m_dataSize, stoFileOSFile::kSeekBegin) != ER_Success)
        return ER_Failure;

    // Go through all of the entries in the global directory and look for ones
    // that are part of this pack file
    int32 sz=0;
    int32 pos=0;
    int32 numEntries=0;
    Directory::iterator directoryIt = directory.begin();
    Directory::iterator directoryEnd = directory.end();
    for (;directoryIt != directoryEnd; ++directoryIt)
    {
        FileEntryVec& entries = (*directoryIt).second;
        FileEntryVec::iterator entryIt = entries.begin();
        FileEntryVec::const_iterator entryEnd = entries.end();

        for (; entryIt != entryEnd; ++entryIt)
        {
            if (entryIt->packFile == this )
            {
                // Found one, so find its bucket in the sorted set, creating
                // one if necessary
                stoPackOSFileEntrySet& set = sortedEntries[(*directoryIt).first];
                set.m_entries.push_back(&(*entryIt));
                numEntries++;
            }
        }
    }
    
    // Account for the deleted entries as well
    numEntries += (int32)(m_deleted.size());

    // Write out the string table
    SortedPackFileEntries::iterator sortedIt = sortedEntries.begin();
    SortedPackFileEntries::iterator sortedEnd = sortedEntries.end();

    for (;sortedIt != sortedEnd; ++sortedIt) 
    {
        const char* pStringValue = g_stringTable.GetString((*sortedIt).first);
        errAssert(pStringValue != NULL);

        sz = (int32)(strlen(pStringValue))+1; //includes NULL char
        m_packFile.Write(pStringValue, sz ); 
        (*sortedIt).second.m_stringOffset = pos;
        pos += sz;
    }

    // create chunk file
    int32 begin = m_packFile.Tell();
    stoViewFile view(m_packFile, begin, 0);
    stoChunkFileWriter writer(view, stoChunkFileWriter::kBinary);

    // write out index info
    int32 stringSize = (int32)pos;
    writer.Put(m_dataSize);
    writer.Put(stringSize);
    writer.Put(numEntries);

    // write out valid entries using a chunk file view
    sortedIt = sortedEntries.begin();
    sortedEnd = sortedEntries.end();
    for (;sortedIt != sortedEnd; ++sortedIt)
    {
        stoPackOSFileEntrySet& set = (*sortedIt).second;
        std::vector<FileEntry*>::iterator entryIt = set.m_entries.begin();
        std::vector<FileEntry*>::iterator entryEnd = set.m_entries.end();

        for (; entryIt != entryEnd; ++entryIt )
        {
            writer.Put((*entryIt)->dataOffset);
            writer.Put((*entryIt)->dataSize);
            writer.Put((*entryIt)->realSize);
            writer.Put(set.m_stringOffset);
            writer.Put((*entryIt)->scheme);
            writer.Put((*entryIt)->modifiedTime);
        }
    }

    // write out deleted entries
    FileEntryVec::iterator it4 = m_deleted.begin();
    FileEntryVec::iterator end4 = m_deleted.end();
    for (;it4 != end4; it4++)
    {
        writer.Put(it4->dataOffset);
        writer.Put(it4->dataSize);
        writer.Put(it4->realSize);
        writer.Put((int32)-1);
        writer.Put(it4->scheme);
        writer.Put(it4->modifiedTime);
    }

    // write out offset to beginning of index
    int32 indexBegin = m_dataSize + stringSize;

    ERR_REPORTV(ES_Info, ("Writing indexBegin <%d>.", indexBegin ));

    m_packFile.Seek(sizeof(int32), stoFileOSFile::kSeekEnd);
    m_packFile.Write(&indexBegin, sizeof(int32));
    return ER_Success;
}

/**
 *  Write a memory block into GLM file.
 *
 *  @param fileName file name to write. It should contain file name portion only, not full path.
 *  @param buffer buffer to write
 *  @param realSize size of buffer
 *  @param entry a reference to a file entry object to fill in
 *
 *  @return true if success or ER_Failure if fail.
**/
errResult stoPackOSFile::Write(const char *fileName, void *buffer, int32 realSize, FileEntry &entry)
{
    int32 paddedSize = BlockSize(realSize);

    // check for empty space to put it in.
    FileEntryVec::iterator it = m_deleted.begin();
    FileEntryVec::iterator end = m_deleted.end();
    FileEntryVec::iterator found = m_deleted.end();
    for (;it != end; it++)
    {
        if (BlockSize(it->dataSize) >= paddedSize)
        {
            // new file fits in this space
            if (found == end)
            {
                found = it;
            } else {
                // find the smallest empty space it fits in.
                if (BlockSize(it->dataSize) < BlockSize(found->dataSize) )
                    found = it;
            }
        }
    }

    if (found != end)
    {
        entry = *found;
        //printf("Writing <%s> into empty space at %d <space sz: %d>\n", fileName, entry.dataOffset, entry.dataSize);
        int32 remainingSpace = BlockSize(found->dataSize) - paddedSize;
        if (remainingSpace == 0)
        {
            // used up all the space in the empty slot
            ERR_REPORTV(ES_Info, ("Removing empty chunk entirely"));
            m_deleted.erase(found);
        }
        else
        {
            // reduce size of empty slot and keep it
            ERR_REPORTV(ES_Info, ("Reducing chunk from %d to %d at %d", found->dataSize, found->dataSize - remainingSpace, found->dataOffset + paddedSize));
            found->dataOffset += paddedSize;
            found->dataSize = remainingSpace;
        }
    } else {
        entry.dataOffset = m_dataSize;
        entry.packFile = this;
        m_dataSize = m_dataSize + paddedSize;
        //corOutputDebugString("Writing <%s> at end of file\n", fileName);
    }
    entry.dataSize = realSize;
    entry.name = g_stringTable.AddString(fileName);

  // write data to file
  m_packFile.Seek(entry.dataOffset, stoFileOSFile::kSeekBegin);
  m_packFile.Write(buffer, realSize);
  if (m_packFile.Tell() != entry.dataOffset + entry.dataSize)
      return ER_Failure;
  
  // pad file to block size
  int32 space = paddedSize - realSize;
  if (space > 0)
  {
      char cleanBlock[stoPackOSFile::BLOCK_SIZE];
      memset(cleanBlock, 0, stoPackOSFile::BLOCK_SIZE);
      m_packFile.Write(cleanBlock, space);
      if (m_packFile.Tell() != entry.dataOffset + paddedSize)
          return ER_Failure;
  }
  return ER_Success;
}

/**
 *  Close the pack file.
 *
 *  @return void
**/
void stoPackOSFile::Close()
{
    //printf("Pack file: %s Closing at %d.\n", m_fileName, m_packFile.GetSize());
    m_packFile.Close();
    return;
}

/**
 *  Delete an entry from the pack file
 *
 *  @parm entry reference to a directory entry
 *  @return ER_Success if the entry is in the pack file, or ER_Failure if not
**/
errResult stoPackOSFile::Delete(FileEntry &entry)
{
    if (entry.packFile != this )
    {
        // entry is not in this pack file!!
        return ER_Failure;
    }
    entry.name = utlStringId::ID_NONE;
    m_deleted.push_back(entry);
    return ER_Success;
}

/**
 *  ListDeleted - print out the deleted/empty slots in the pack file
 *
 *  @return total size of empty space
**/
int32 stoPackOSFile::ListDeleted()
{
    int32 sz = 0;
    // dump list of deleted slots
    FileEntryVec::iterator it = m_deleted.begin();
    FileEntryVec::iterator end = m_deleted.end();
    for (;it != end; it++)
    {
        FileEntry e = *it;
        printf("    Deleted: position: <%8d> size: %8d\n", e.dataOffset, e.dataSize);
        sz += it->dataSize;
    }
    return sz;
}

bool FileEntryCmp(const FileEntry &e1, const FileEntry &e2)
{
    return _stricmp(g_stringTable.GetString(e1.name), g_stringTable.GetString(e2.name)) < 0;
}

bool FileEntryNameCmp(const FileEntry &e1, const char *fileName)
{
  return _stricmp(g_stringTable.GetString(e1.name), fileName) < 0;
}

bool FileEntryNameIdCmp(const FileEntry &e1, const utlStringId& fileName)
{
    return _stricmp(g_stringTable.GetString(e1.name), g_stringTable.GetString(fileName)) < 0;
}

/**
 *  Get file name portion from path. Path separator
 *  should be '/'. 
 *
 *  @param path path to a file
 *  @param fileName (out) file name extracted
**/
void GetFileName(const char *path, char *fileName, const char *baseDir)
{
    if (baseDir)
    {
        // strip off base dir if there is one
        size_t len = strlen(baseDir);
        if (_strnicmp(baseDir, path, len) == 0)
        {
            // matches the base dir - only use remaining piece
            const char *p = (path+len);
            while (*p == '/' || *p == '\\')
                p++;
            strcpy(fileName, p);
            return;
        }
        else
        {
            // keep the existing path
            strcpy(fileName, path);
            return;
        }
    }

  const char *ptr = strrchr(path, '/');
  if (ptr)
  {
    strcpy(fileName, ptr + 1);
  }
  else
  {
      ptr = strrchr(path, '\\');
      if (ptr)
      {
          strcpy(fileName, ptr + 1);
      }
      else
      {
        strcpy(fileName, path);
      }
  }
}

/**
 *  Calculate the size in blocks of a size in bytes. 
 *
 *  @param size the size in bytes to be converted.
 *  @return the size in blocks
 *
**/
int32 stoPackOSFile::BlockSize(int32 size)
{
    return (size + (stoPackOSFile::BLOCK_SIZE - 1)) & ~(stoPackOSFile::BLOCK_SIZE - 1);
}

bool stoPackOSFile::IsFile(stoFileOSFile *file) const
{
    return file == &m_packFile;
}
