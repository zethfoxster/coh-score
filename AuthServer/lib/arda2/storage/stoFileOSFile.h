/*****************************************************************************
    created:    2001/05/24
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese

    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_stoFileOSFile
#define   INCLUDED_stoFileOSFile
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/storage/stoFile.h"
#include "arda2/core/corStlVector.h"
#include "arda2/util/utlEnumOperators.h"

class stoFileOSFile : public stoFile
{
public:

    enum AccessMode
    {
        kAccessNone         = 0x0,
        kAccessRead         = 0x1,
        kAccessWrite        = 0x2,
        kAccessReadWrite    = kAccessRead | kAccessWrite,
        kAccessCreate       = 0x4 | kAccessWrite,
    };

    stoFileOSFile();
    virtual ~stoFileOSFile();

    errResult Open( const char* filename, AccessMode mode, bool textFlag = false );
    errResult Close();

    // Seek - moves the file pointer to the specified offset
    virtual errResult   Seek( int offset, SeekMode mode = kSeekBegin );
    
    // Rewind - typically same as Seek() to beginning
    virtual errResult   Rewind();
    
    // Flush - flush the buffer 
    virtual errResult   Flush();
    
    // Eof - check for end of file
    virtual bool        Eof() const;
    
    // Read - read 'size' bytes into buffer from the file
    virtual errResult   Read(void* buffer, size_t size);
    
    // Write - writes 'size' byte from the buffer into the file
    virtual errResult   Write(const void* buffer, size_t size);
    
    virtual int         GetSize() const;
    
    // IsValid - returns true when the file is valid (open)
    virtual bool        IsValid() const;
    
    virtual bool        CanRead() const;
    
    virtual bool        CanWrite() const;
    
    virtual bool        CanSeek() const;
    
    // Tell - returns the current offset into the file
    virtual int         Tell() const;

    void * AcquireMemoryBuffer(OUT uint &bufferSize);
    void ReleaseMemoryBuffer();

private:
    AccessMode          m_accessMode;
    int                 m_hFile;
    byte                *m_tempBuffer;
    int                 m_tempBufferRefCount;
};


MAKE_ENUM_OPERATORS(stoFileOSFile::AccessMode);


#endif // INCLUDED_stoFileOSFile
