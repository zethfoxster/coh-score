/*****************************************************************************
    created:    2001/05/24
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese

    purpose:    OS File implementation of file object
*****************************************************************************/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFileOSFile.h"
#include "arda2/storage/stoFileUtils.h"

#include <errno.h>
#include <sys/types.h>

stoFileOSFile::stoFileOSFile() :
    m_accessMode(kAccessNone),
    m_hFile(-1),
    m_tempBuffer(NULL),
    m_tempBufferRefCount(0)
{
}


stoFileOSFile::~stoFileOSFile()
{
    //should this delete the buffer if refcount is > 0??
    if (m_tempBuffer)
        delete [] m_tempBuffer;

    if ( IsValid() )
    {
        Close();
    }
}


errResult stoFileOSFile::Open( const char* filename, AccessMode mode, bool textFlag )
{
    int flags = 0;
// Opening a file as "text" or "binary" only makes a difference in Windows
#ifdef CORE_SYSTEM_WINAPI
    if (textFlag)
    {
        flags = _O_TEXT;
    }
    else
    {
        flags = _O_BINARY;
    }
#endif
    int filemode = 0;

    switch ( mode )
    {
    case kAccessRead:
        flags |= _O_RDONLY;
        break;

    case kAccessWrite:
        flags |= _O_WRONLY;
        break;

    case kAccessReadWrite:
        flags |= _O_RDWR;
        break;

    case kAccessCreate:
        flags |= _O_WRONLY | _O_CREAT | _O_TRUNC;
        filemode |= (_S_IWRITE | _S_IREAD);
        break;

    case kAccessNone:
	    return ER_Failure;
    }

    m_accessMode = mode;

    m_hFile = _open(filename, flags, filemode);

    if ( m_hFile == -1 )
    {
        switch (errno)
        {
        case EACCES:
            ERR_REPORTV(ES_Error, ("Tried to open read-only file for writing, or file's sharing mode does not allow specified operations, or given path is directory."));
            break;
        case EEXIST:
            ERR_REPORTV(ES_Error, ("_O_CREAT and _O_EXCL flags specified, but filename already exists."));
            break;
        case EINVAL:
            ERR_REPORTV(ES_Error, ("Invalid oflag or pmode argument."));
            break;
        case EMFILE:
            ERR_REPORTV(ES_Error, ("No more file descriptors available (too many open files)."));
            break;
        case ENOENT:
#if CORE_SYSTEM_XENON
            ERR_REPORTV(ES_Error, ("File not found: <%s>", filename ));
#else
            char buf[_MAX_PATH];
            getcwd(buf, _MAX_PATH);
            ERR_REPORTV(ES_Error, ("File or path not found: <%s> <%s>", buf,filename ));
#endif
            break;
        }
        return ER_Failure;
    }

    return ER_Success;
}


errResult stoFileOSFile::Close()
{
    if ( IsValid() )
    {
        _close(m_hFile);
    }

    m_hFile = -1;

    return ER_Success;
}


// Seek - moves the file pointer to the specified offset
errResult stoFileOSFile::Seek( int offset, SeekMode mode )
{
    int whence = SEEK_CUR;
    switch ( mode )
    {
    case kSeekBegin:
        whence = SEEK_SET;
        break;

    case kSeekCurrent:
        whence = SEEK_CUR;
        break;

    case kSeekEnd:
        whence = SEEK_END;
        break;
    }

    _seek_offset newPosition = _lseek(
        m_hFile,    // must have GENERIC_READ and/or GENERIC_WRITE 
        offset,     // bytes to move pointer
        whence);    // provides offset from current position 

    if ( newPosition == -1 )
        return ER_Failure;

    return ER_Success;
}


// Rewind - typically same as Seek() to beginning
errResult stoFileOSFile::Rewind()
{
    return Seek(0, kSeekBegin);
}


// Flush - flush the buffer 
errResult stoFileOSFile::Flush()
{
    ERR_RETURN_UNIMPLEMENTED();
}


// Eof - check for end of file
bool stoFileOSFile::Eof() const
{
#ifdef WIN32
    return eof(m_hFile) != 0;
#else
    return Tell() == GetSize();
#endif
}


// Read - read 'size' bytes into buffer from the file
errResult stoFileOSFile::Read( void* buffer, size_t size )
{
    int bytesRead = _read(m_hFile, buffer, static_cast<uint32>(size));

    if ( (bytesRead != -1) && (static_cast<uint32>(bytesRead) == static_cast<uint32>(size)) )
        return ER_Success;

    return ER_Failure;
}


// Write - writes 'size' byte from the buffer into the file
errResult stoFileOSFile::Write(const void* buffer, size_t size )
{
    int bytesWritten = _write(
        m_hFile,                 // handle to file
        buffer,                  // data buffer
        static_cast<uint32>(size)// number of bytes to write (clamped to 32-bits)
        );

    if ( (bytesWritten != -1) && (static_cast<uint32>(bytesWritten) == static_cast<uint32>(size)) )
        return ER_Success;

    return ER_Failure;
}


int stoFileOSFile::GetSize() const
{
#ifdef CORE_SYSTEM_WINAPI
    return _filelength(m_hFile);
#elif CORE_SYSTEM_PS3
    struct CellFsStat statResult;
    if (cellFsFstat(m_hFile, &statResult)==CELL_FS_SUCCEEDED)
        return statResult.st_size;
    else
        return -1;
#else
    struct _stat statResult;
    _fstat(m_hFile, &statResult);
    return statResult.st_size;
#endif
}


// IsValid - returns true when the file is valid (open)
bool stoFileOSFile::IsValid() const
{
    return m_hFile != -1;
}


bool stoFileOSFile::CanRead() const
{
    return (m_accessMode & kAccessRead) != 0;
}


bool stoFileOSFile::CanWrite() const
{
    return (m_accessMode & kAccessWrite) != 0;
}


bool stoFileOSFile::CanSeek() const
{
    return true;
}


// Tell - returns the current offset into the file
int stoFileOSFile::Tell() const
{
    _seek_offset dwCurrentFilePosition = _lseek( 
        m_hFile,        // must have GENERIC_READ and/or GENERIC_WRITE 
        0,              // do not move pointer 
        SEEK_CUR);  // provides offset from current position 

    return (int)dwCurrentFilePosition;
}

void *stoFileOSFile::AcquireMemoryBuffer(OUT uint &bufferSize)
{
    if (m_tempBufferRefCount > 0)
    {
        m_tempBufferRefCount++;
    }
    else
    {
        uint bufferSize = GetSize();
        m_tempBuffer = new byte[bufferSize];
        if (ISOK(Read(m_tempBuffer, bufferSize)) )
        {
            m_tempBufferRefCount++;
        } else {
            delete [] m_tempBuffer;
            m_tempBuffer = NULL;
        }
    }
    bufferSize = GetSize();
    return m_tempBuffer;
}

void stoFileOSFile::ReleaseMemoryBuffer()
{
    if (m_tempBufferRefCount > 1)
    {
        m_tempBufferRefCount--;
    }
    else
    {
        if (m_tempBuffer)
        {
            delete [] m_tempBuffer;
            m_tempBuffer = NULL;
        }
    }
}

