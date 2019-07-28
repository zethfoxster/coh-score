/*****************************************************************************
	created:	2002/10/22
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_stofilememorymap_h
#define   INCLUDED_stofilememorymap_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_SYSTEM_WINAPI

#include "arda2/storage/stoFileOSFile.h"

class stoFileMemoryMap : stoFile
{
public:

	enum AccessMode
	{
		kAccessNone			= 0x0,
		kAccessRead			= 0x1,
		kAccessWrite		= 0x2,
		kAccessReadWrite	= kAccessRead | kAccessWrite,
		kAccessCreate		= 0x4 | kAccessWrite,
	};

						stoFileMemoryMap();
						~stoFileMemoryMap();

	virtual errResult	Open( const char* filename, AccessMode mode );
	virtual errResult	Close();

	uint32				GetFileProtect(AccessMode am) const;
	
	// Seek - moves the file pointer to the specified offset
	virtual errResult	Seek( int offset, SeekMode mode = kSeekBegin );

	// Rewind - typically same as Seek() to beginning
	virtual errResult	Rewind();

	// Flush - flush the buffer 
	virtual errResult	Flush();

	// Eof - check for end of file
	virtual bool		Eof() const;

	// Read - read 'size' bytes into buffer from the file
	virtual errResult	Read(void* buffer, size_t size);

	// Write - writes 'size' byte from the buffer into the file
	virtual errResult	Write(const void* buffer, size_t size);

	// IsValid - returns true when the file is valid (open)
	virtual bool		IsValid() const;

	// Tell - returns the current offset into the file
	virtual int			Tell() const;

protected:
	
private:
	// disable copy constructor and assignment operator
	stoFileMemoryMap( const stoFileMemoryMap& );
	stoFileMemoryMap& operator=( const stoFileMemoryMap& );

	HANDLE m_hFile;
	HANDLE	m_hMemoryMap;
	uint	m_FileSize;
	byte*	m_pEndPos;
	byte*	m_pStartPos;
	byte*   m_pCurrentPos;
};

inline bool stoFileMemoryMap::Eof() const
{
	return (m_pCurrentPos == m_pEndPos);
}

inline errResult stoFileMemoryMap::Read(void* buffer, size_t size)
{
	byte *pDest = (byte*)buffer;
	
	if(Eof())
		return ER_Failure;

	memcpy(pDest, m_pCurrentPos, size);
	m_pCurrentPos += size;
	return ER_Success;
}

#endif // CORE_SYSTEM_WINAPI

#endif // INCLUDED_stofilememorymap_h
