/*****************************************************************************
	created:	2002/04/02
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	An stoFileMemory object maintains a resizable memory buffer. 
				Writing to the file will automatically increase the size of the 
				buffer at the expense of some periodic reallocation.
*****************************************************************************/

#ifndef   INCLUDED_stoFileMemory_h
#define   INCLUDED_stoFileMemory_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/storage/stoFile.h"

class stoFileMemory : public stoFile
{
public:
	explicit stoFileMemory( uint reserveSize = 1024 );
	explicit stoFileMemory( void *buffer, uint size );
	explicit stoFileMemory( const void *buffer, uint size );

	~stoFileMemory();

	// Seek - moves the file pointer to the specified offset
	virtual errResult Seek( int offset, SeekMode mode = kSeekBegin );

	// Rewind - typically same as Seek() to beginning
	virtual errResult Rewind();

	// Flush - flush the buffer 
	virtual errResult Flush();

    // reset file for reuse
	virtual void Reset();

	// Eof - check for end of file
	virtual bool Eof() const;

    // Utility interface functions

    // Visual Studio seems to be braindead and refuses to acknowledge the existence of these
    // template member functions from the base class. So they are redeclared here.
    template <typename T>
    errResult	Read( T& t)
    { return Read(&t, sizeof(T)); }

    template <typename T>
    errResult	Write( const T& t )
    { return Write(&t, sizeof(T)); }

	// Read - read 'size' bytes into buffer from the file
	virtual errResult Read( void* buffer, size_t size );

	// Write - writes 'size' byte from the buffer into the file
	virtual errResult Write( const void* buffer, size_t size );

	virtual int GetSize() const;

	// IsValid - returns true when the file is valid (open)
	virtual bool IsValid() const;

	virtual bool CanRead() const;

	virtual bool CanWrite() const;

	virtual bool CanSeek() const;

	// Tell - returns the current offset into the file
	virtual int	Tell() const;

    // these methods are dangerous! probably should be friended only
	void * GetBuffer() const { return m_pBuffer; }
	void SetBufferSize(int32 size) { m_size = size;}

    void * AcquireMemoryBuffer(OUT uint &bufferSize);
    void ReleaseMemoryBuffer();

protected:
	void ReallocateBuffer();

private:
	byte	*m_pBuffer;
	int		m_position;
	int		m_size;
	int		m_reserve;
	bool	m_bOwnsBuffer;
	bool	m_bCanWrite;
};


#endif // INCLUDED_stoFileMemory_h
