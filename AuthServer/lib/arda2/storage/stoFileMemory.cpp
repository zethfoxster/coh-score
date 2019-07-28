/*****************************************************************************
	created:	2002/04/02
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	An stoFileMemory object maintains a resizable memory buffer. 
				Writing to the file will automatically increase the size of the 
				buffer at the expense of some periodic reallocation.
*****************************************************************************/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFileMemory.h"

stoFileMemory::stoFileMemory( uint reserveSize ) :
	m_pBuffer(NULL),
	m_position(0),
	m_size(0),
	m_reserve(reserveSize),
	m_bOwnsBuffer(true),
	m_bCanWrite(true)
{
	ReallocateBuffer();
}


stoFileMemory::stoFileMemory( void *pBuffer, uint size ) :
	m_pBuffer((byte *)pBuffer),
	m_position(0),
	m_size(size),
	m_reserve(size),
	m_bOwnsBuffer(false),
	m_bCanWrite(true)
{
}


stoFileMemory::stoFileMemory( const void *pBuffer, uint size ) :
	m_pBuffer((byte *)pBuffer),
	m_position(0),
	m_size(size),
	m_reserve(size),
	m_bOwnsBuffer(false),
	m_bCanWrite(false)
{
}


stoFileMemory::~stoFileMemory()
{
	if ( m_bOwnsBuffer )
		free(m_pBuffer);
}


// Seek - moves the file pointer to the specified offset
errResult stoFileMemory::Seek( int offset, SeekMode mode )
{
	switch ( mode )
	{
		case kSeekBegin:
			if ( offset > m_size || offset < 0 )
				return ER_Failure;
			m_position = offset;
			return ER_Success;

		case kSeekCurrent:
			if ( m_position + offset > m_size || m_position + offset < 0 )
				return ER_Failure;
			m_position += offset;
			return ER_Success;

		case kSeekEnd:
			if ( offset > 0 || m_size + offset < 0 )
				return ER_Failure;
			m_position = m_size + offset;
			return ER_Success;
	}
	return ER_Failure;
}


// Rewind - typically same as Seek() to beginning
errResult stoFileMemory::Rewind()
{
	m_position = 0;
	return ER_Success;
}


// Flush - flush the buffer 
errResult stoFileMemory::Flush()
{
	return ER_Success;
}


// reset file for reuse
void stoFileMemory::Reset()
{
    m_size=0;
    m_position=0;
}


// Eof - check for end of file
bool stoFileMemory::Eof() const
{
	return m_position == m_size;
}


// Read - read 'size' bytes into buffer from the file
errResult stoFileMemory::Read(void* buffer, size_t size)
{
	if ( m_position + (int)size > m_size )
		return ER_Failure;

	memcpy(buffer, &m_pBuffer[m_position], size);
	m_position += (int32)size;
	return ER_Success;
}


// Write - writes 'size' byte from the buffer into the file
errResult stoFileMemory::Write(const void* buffer, size_t size)
{
	if ( !m_bCanWrite )
		return ER_Failure;

	if ( m_position + (int)size > m_reserve )
	{
		if ( !m_bOwnsBuffer )
			return ER_Failure;

        do 
        {
			// grow geometrically using phi approximation
            m_reserve = m_reserve * 103 / 64;
        }
        while ( m_position + (int)size > m_reserve );

		ReallocateBuffer();
	}

	memcpy(&m_pBuffer[m_position], buffer, size);
	m_position += (int32)size;
	if (m_position > m_size )
		m_size = m_position;

	return ER_Success;
}


int stoFileMemory::GetSize() const
{
	return m_size;
}


// IsValid - returns true when the file is valid (open)
bool stoFileMemory::IsValid() const
{
	return true;
}


bool stoFileMemory::CanRead() const
{
	return true;
}


bool stoFileMemory::CanWrite() const
{
	return m_bCanWrite;
}


bool stoFileMemory::CanSeek() const
{
	return true;
}

// Tell - returns the current offset into the file

int	stoFileMemory::Tell() const
{
	return m_position;
}

void stoFileMemory::ReallocateBuffer()
{
	m_pBuffer = (byte *)realloc(m_pBuffer, m_reserve);
}

void *stoFileMemory::AcquireMemoryBuffer(OUT uint &bufferSize)
{
    bufferSize = m_size;
    return m_pBuffer;
}

void stoFileMemory::ReleaseMemoryBuffer()
{
    // do nothing
}
