/*****************************************************************************
	created:	2002/10/01
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	Buffered interface for stoFile objects. This exposed the same
				interface functions, but is non-derived (no virtual functions)
				for performance. This currently exposes only the interface
				required for reading chunk files.
*****************************************************************************/

#ifndef   INCLUDED_stoFileBuffer_h
#define   INCLUDED_stoFileBuffer_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/storage/stoFile.h"
#include "arda2/util/utlBlockAllocator.h"
static const int iBufferSize = 16*1024;

class stoFileBuffer : public utlBlockAllocator<stoFileBuffer,16,16>
{
public:
	stoFileBuffer(stoFile &file) : 
	    m_file(file),
	    m_bufferPos(0),
	    m_bufferFilePos(0),
	    m_bufferLength(0),
	    m_bufferAllocSize(iBufferSize),
	    m_fileSize(0)
	{
//        ERR_REPORTV(ES_Info, ("Allocating stoFileBuffer - %dk bytes", iBufferSize/1024));
		m_fileSize = m_file.GetSize();
	}

	~stoFileBuffer()
	{
	}

	// Seek - moves the file pointer to the specified offset
	errResult Seek( int offset, stoFile::SeekMode mode = stoFile::kSeekBegin )
	{
		switch ( mode )
		{
			case stoFile::kSeekBegin:
				break;
			case stoFile::kSeekCurrent:
				offset += m_bufferFilePos + m_bufferPos;
				break;
			case stoFile::kSeekEnd:
				offset = m_fileSize - offset;
				break;
			default:
				ERR_RETURN_UNIMPLEMENTED();
		}
		if ( offset < 0 || offset > m_fileSize )
			return ER_Failure;

		// can seek directly within buffer?
		if ( offset >= m_bufferFilePos && offset < m_bufferFilePos + m_bufferLength )
		{
			m_bufferPos = offset - m_bufferFilePos;
			return ER_Success;
		}

		// invalidate buffer and mark new file position (seek will be deferred until next read)
		m_bufferFilePos = offset;
		m_bufferPos = m_bufferLength = 0;
		return ER_Success;
	}

	// Rewind - typically same as Seek() to beginning
	errResult	Rewind()
	{
		return Seek(0, stoFile::kSeekBegin);
	}

	// Eof - check for end of file
	bool		Eof() const
	{
		return m_bufferFilePos + m_bufferPos >= m_fileSize;
	}

	int			GetSize() const
	{
		return m_fileSize;
	}

	// IsValid - returns true when the file is valid (open)
	bool		IsValid() const
	{
		return m_file.IsValid();
	}

	bool		CanRead() const { return m_file.CanRead(); }

	bool		CanWrite() const { return false; }

	bool		CanSeek() const	{ return true; }

	// Tell - returns the current offset into the file
	int			Tell() const
	{
		return m_bufferFilePos + m_bufferPos;
	}

	errResult	ReadBuffer( int iPos )
	{
		// beginning of block already in buffer?
		if ( iPos >= m_bufferFilePos && iPos < m_bufferFilePos + m_bufferLength )
		{
			// move existing block to the start
			int iBlockStart = iPos - m_bufferFilePos;
			int iBytesInBuffer = m_bufferLength - iBlockStart;
			memmove(&m_pBuffer[0], &m_pBuffer[iBlockStart], iBytesInBuffer);

			m_bufferFilePos = iPos;
			m_bufferLength = Min(m_bufferAllocSize, m_fileSize - m_bufferFilePos);
			m_bufferPos = 0;

			// read missing buffer end
			int iBytesToRead = m_bufferLength - iBytesInBuffer;
			m_file.Seek(m_bufferFilePos + iBytesInBuffer);
			return m_file.Read(&m_pBuffer[iBytesInBuffer], iBytesToRead);
		}

		// end of block already in buffer?
		if ( iPos < m_bufferFilePos && iPos + m_bufferAllocSize >= m_bufferFilePos )
		{
			// move existing block to the end
			int iBytesInBuffer = Min(iPos + m_bufferAllocSize - m_bufferFilePos, m_bufferLength);
			int iMoveDist = m_bufferFilePos - iPos;
			memmove(&m_pBuffer[iMoveDist], &m_pBuffer[0], iBytesInBuffer);

			m_bufferFilePos = iPos;
			m_bufferLength = Min(m_bufferAllocSize, m_fileSize - m_bufferFilePos);
			m_bufferPos = 0;

			// read missing buffer start
			m_file.Seek(m_bufferFilePos);
			return m_file.Read(&m_pBuffer[0], iMoveDist);
		}

		// read in an entire new buffer
		m_bufferFilePos = iPos;
		m_bufferPos = 0;
		m_file.Seek(m_bufferFilePos);

		// FIXME[pmf]: this would be easier with a file read function that returns
		// the number of bytes read
		m_bufferLength = Min(m_bufferAllocSize, m_fileSize - m_bufferFilePos);
		return m_file.Read(m_pBuffer, m_bufferLength);
	}

	template <int nLength>
	bool		CheckBufferRead()
	{
		// need to read in a new buffer?
		if ( m_bufferPos + nLength > m_bufferLength )
		{
			// align the new buffer to the beginning of the block
			if ( !ISOK(ReadBuffer(m_bufferFilePos + m_bufferPos)) )
				return false;

			if ( nLength > m_bufferLength )
				return false;
		}
		return true;
	}

	// Utility interface functions
	template <typename T>
	errResult			Read( T& t)
	{
		if ( !CheckBufferRead<sizeof(T)>() )
			return ER_Failure;

		memcpy(&t, &m_pBuffer[m_bufferPos], sizeof(T));
		m_bufferPos += sizeof(T);
		return ER_Success;
	}

	errResult	ReadLine( std::string &s )
    {
	    s.resize(0);
	    bool lastwascr = false;

	    while ( !Eof() )
	    {
		    char c;
		    errResult r = Read(c);
		    if ( !ISOK(r) )
			    return r;
		    if ( c == '\n' )
			    break;

		    if(lastwascr)
		    { 
			    // if the last char was a cr, and we made it here, 
			    // the next char was not a lf, so back up by one	
			    errAssert( CanSeek() ); // catch cases where we can't seek...
                Seek(-1, stoFile::kSeekCurrent);
			    break;
		    }

		    if ( c == '\r' ){
			    lastwascr = true;
			    continue;
		    }

		    s += c;
	    }
	    return ER_Success;
    }

	// Read - read 'size' bytes into buffer from the file
	errResult	Read(void* buffer, int size)
	{
		if ( m_bufferPos + size > m_bufferLength )
		{
            // if block is larger than buffer, read directly
			if ( size > m_bufferAllocSize )
			{
                // beginning of block already in buffer?
                if ( m_bufferPos < m_bufferLength )
                {
                    // copy what's already available in buffer
                    int iBytesInBuffer = m_bufferLength - m_bufferPos;
                    memcpy(buffer, &m_pBuffer[m_bufferPos], iBytesInBuffer);
                    m_bufferPos += iBytesInBuffer;

                    // offset destination and decrease the amount we need to read
                    buffer = (byte *)buffer + iBytesInBuffer;
                    size -= iBytesInBuffer;
                }

				m_file.Seek(m_bufferFilePos + m_bufferPos);
				m_file.Read(buffer, size);
				m_bufferPos += size;
				return ER_Success;
			}

			// just need to read in a new buffer
			if ( !ISOK(ReadBuffer(m_bufferFilePos + m_bufferPos)) )
				return ER_Failure;
		}
		memcpy(buffer, &m_pBuffer[m_bufferPos], size);
		m_bufferPos += size;
		return ER_Success;
	}

	//template<>
	errResult			Read( std::string & s )
	{
		return ReadString(s);
	}

	errResult			ReadString( std::string &s )
	{
		// read the string, up to and including the trailing \0
		s.resize(0);

		while ( !Eof() )
		{
			char c;
			errResult r = Read(c);
			if ( !ISOK(r) )
				return r;

			if ( c == '\0' )
				return ER_Success;

			s += c;
		}
		return ER_Failure;
	}

    void * AcquireMemoryBuffer(OUT uint &bufferSize)
    {
        ReadBuffer(m_fileSize); // ensure all of file is in memory
        bufferSize = m_fileSize;
        return m_pBuffer;
    }

    void ReleaseMemoryBuffer()
    {
        // do nothing.
    }

private:
	// disable copy constructor and assignment
	stoFileBuffer( const stoFileBuffer & );
	stoFileBuffer& operator=( const stoFileBuffer& );

	stoFile				&m_file;
	int					m_bufferPos;
	int					m_bufferFilePos;
	int					m_bufferLength;
	int					m_bufferAllocSize;
	int					m_fileSize;

    byte				m_pBuffer[iBufferSize];
};


#endif // INCLUDED_stoFile_h
