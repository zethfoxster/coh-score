/*****************************************************************************
	created:	2002/04/02
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	An stoViewFile is a view into another file object. All file
				operations are deferred through the interface to the
				containing file object.

				As the stoViewFile may share its file descriptor - and so file 
				seek location - with other file objects, it tracks the seek position
				and resets it before any file operation is made. This incurs a 
				performance hit, but ensures that the file operations are always
				being performed in the write location in the file.
				(SR 1/20/03)

*****************************************************************************/

#ifndef   INCLUDED_stoViewFile_h
#define   INCLUDED_stoViewFile_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/storage/stoFile.h"

class stoViewFile : public stoFile
{
public:
	explicit stoViewFile( stoFile &file, uint offset, uint size, bool autoSeek=true ) :
		m_file(file),
		m_size(size),
		m_offset(offset),
		m_autoSeek(autoSeek),
		m_seekLocation(offset),
        m_tempBuffer(NULL),
        m_tempBufferRefCount(0)
	{
	}

    ~stoViewFile()
    {
        //should this delete the buffer if refcount is > 0??
        if (m_tempBuffer)
            delete [] m_tempBuffer;
    }

	// Seek - moves the file pointer to the specified offset
	virtual errResult	Seek( int offset, SeekMode mode = kSeekBegin )
	{
		if (!ISOK(AutoSeek()) )
			return ER_Failure;

		errResult result = ER_Failure;
		switch ( mode )
		{
		case kSeekBegin:
			{
				result = m_file.Seek(m_offset + offset, kSeekBegin);
				if ( ISOK(result) )
					this->SetSeekLocation(m_offset + offset);
				break;
			}

		case kSeekCurrent:
			{
                result = m_file.Seek(offset, kSeekCurrent);
				if ( ISOK(result) )
					this->SetSeekLocation(m_seekLocation + offset);
				break;
			}

		case kSeekEnd:
			{
				result = m_file.Seek(m_offset + m_size + offset, kSeekBegin);
				if ( ISOK(result) )
					this->SetSeekLocation(m_offset + m_size + offset);
				break;
			}
		}
		return result;
	}

	// Rewind - typically same as Seek() to beginning
	virtual errResult	Rewind()
	{
		if (!ISOK(AutoSeek()) )
			return ER_Failure;

		errResult result = m_file.Seek(m_offset, kSeekBegin);
		if ( ISOK(result) )
			this->SetSeekLocation(m_offset);
		return result;
	}

	// Flush - flush the buffer 
	virtual errResult	Flush()
	{
		if (!ISOK(AutoSeek()) )
			return ER_Failure;

		return m_file.Flush();
	}

	// Eof - check for end of file
	virtual bool		Eof() const
	{
		return m_file.Tell() >= m_offset + m_size;
	}

	// Read - read 'size' bytes into buffer from the file
	virtual errResult	Read(void* buffer, size_t size)
	{
		if (!ISOK(AutoSeek()) )
			return ER_Failure;

		errResult result = m_file.Read(buffer, size);
		if ( ISOK(result) )
			this->SetSeekLocation((int32)(m_seekLocation + size)); // Safely assume files will be smaller than 4GB in Palantir?
		return result;
	}

	// Write - writes 'size' byte from the buffer into the file
	virtual errResult	Write(const void* buffer, size_t size)
	{
		if (!ISOK(AutoSeek()) )
			return ER_Failure;

		errResult result = m_file.Write(buffer, size);
		if ( ISOK(result) )
			this->SetSeekLocation((int32)(m_seekLocation + size)); // Safely assume files will be smaller than 4GB in Palantir?
		return result;

	}

	virtual int			GetSize() const
	{
		return m_size;
	}

	// IsValid - returns true when the file is valid (open)
	virtual bool		IsValid() const
	{
		return m_file.IsValid();
	}

	virtual bool		CanRead() const
	{
		return m_file.CanRead();
	}

	virtual bool		CanWrite() const
	{
		return m_file.CanWrite();
	}

	virtual bool		CanSeek() const
	{
		return m_file.CanSeek();
	}

	// Tell - returns the current offset into the file
	virtual int			Tell() const
	{
		// NOTE: this should be the same as m_seekLocation...
		return m_file.Tell() + m_offset;
	}

    void * AcquireMemoryBuffer(OUT uint &bufferSize)
    {
        // this does not take advantage of the case where the view
        // is into a memory file...
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

    void ReleaseMemoryBuffer()
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

private:

	// disable copy constructor and assignment
	stoViewFile( const stoViewFile & );
	stoViewFile& operator=( const stoViewFile& );

	inline errResult AutoSeek()
	{
		if (this->m_autoSeek)
			return this->m_file.Seek(m_seekLocation, kSeekBegin);
		return ER_Success;
	}
	inline void SetSeekLocation(int32 newLocation)
	{
		m_seekLocation = newLocation;
	}

	stoFile &m_file;
	int32	m_size;
	int32	m_offset;
	bool    m_autoSeek;
	int32   m_seekLocation;	// seek location (location in m_file, not in this view)
    byte    *m_tempBuffer;
    int     m_tempBufferRefCount;
};



#endif // INCLUDED_stoViewFile_h
