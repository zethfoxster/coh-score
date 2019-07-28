/*****************************************************************************
	created:	2002/10/22
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	
*****************************************************************************/
#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFileMemoryMap.h"

#if CORE_SYSTEM_WINAPI && !(CORE_SYSTEM_XENON)

stoFileMemoryMap::stoFileMemoryMap():	m_pEndPos(0),
										m_pStartPos(0), 
										m_pCurrentPos(0), 
										m_hFile(INVALID_HANDLE_VALUE),
										m_hMemoryMap(INVALID_HANDLE_VALUE), 
										m_FileSize(0)
{

}

stoFileMemoryMap::~stoFileMemoryMap()
{
	Close();
}

uint32 stoFileMemoryMap::GetFileProtect(AccessMode am) const
{
	if(am & kAccessWrite)
		return PAGE_READWRITE;
	return PAGE_READONLY;
}

errResult stoFileMemoryMap::Open( const char* filename, AccessMode mode )
{
	errAssert(!(mode&kAccessWrite)); // write mode not supported yet

	DWORD dwDesiredAccess = 0;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwShareMode = 0;
	switch ( mode )
	{
		case kAccessRead:
			dwDesiredAccess = GENERIC_READ;
			dwShareMode = FILE_SHARE_READ;
			break;

		case kAccessWrite:
			dwDesiredAccess = GENERIC_WRITE;
			break;

		case kAccessReadWrite:
			dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
			break;

		case kAccessCreate:
			dwDesiredAccess = GENERIC_WRITE;
			dwCreationDisposition = CREATE_ALWAYS;
			break;
	}

	m_hFile = CreateFile(
		filename, 
		dwDesiredAccess, 
		dwShareMode, 
		NULL, 
		dwCreationDisposition, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
		NULL);

	if ( m_hFile == INVALID_HANDLE_VALUE )
		return ER_Failure;

	m_FileSize = 2 * 1024 * 1024; // default to 2 MB
	bool bReadOnly = false;
	if(mode & kAccessRead)
	{
		// size to the file length exactly...
		m_FileSize = GetSize();
		bReadOnly = true;
	}
	
	// now try to open the memory mapping
	m_hMemoryMap = CreateFileMapping(m_hFile, 0, GetFileProtect(mode), 0, 0, 0);
	if(m_hMemoryMap)
	{
		m_pCurrentPos = m_pStartPos = (byte*)
			MapViewOfFile(m_hMemoryMap, bReadOnly?FILE_MAP_READ:FILE_MAP_WRITE, 0, 0, m_FileSize);
		m_pEndPos = m_pStartPos + m_FileSize;
		return ER_Success;
	}
	
	// boo, this failed
	Close();
	return ER_Failure;
}

errResult stoFileMemoryMap::Close()
{
	if(m_pStartPos)
	{
		UnmapViewOfFile(m_pStartPos);
		m_pEndPos = m_pStartPos = m_pCurrentPos = 0;
	}

	if(m_hMemoryMap)
	{
		CloseHandle(m_hMemoryMap);
		m_hMemoryMap = INVALID_HANDLE_VALUE;
	}
	m_FileSize = 0;

	if (IsValid())
	{
		CloseHandle(m_hFile);
	}
	m_hFile = INVALID_HANDLE_VALUE;

	return ER_Success;
}

errResult stoFileMemoryMap::Seek( int offset, SeekMode mode )
{
	byte* pLoc=(byte*)0;

	switch( mode )
	{
		case kSeekBegin:
			pLoc = m_pStartPos + offset;
		break;
		case kSeekCurrent:
			pLoc = m_pCurrentPos + offset;
		break;
		case kSeekEnd:
			pLoc = m_pStartPos + m_FileSize + offset;
		break;

		// musta added a new type..., we no comprende
		default:
			return ER_Failure;
	}

	// catch attempts to move past end of file
	if(pLoc > ((byte*)m_pStartPos + m_FileSize))
	{
		// this would result in an access violation for memory mapped files
		// we could automagically expand the size and return
		return ER_Failure;
	}
	else
	{
		m_pCurrentPos = pLoc;
	}

	return ER_Success;
}

errResult stoFileMemoryMap::Rewind()
{
	m_pCurrentPos = m_pStartPos;
	return ER_Failure;
}

errResult stoFileMemoryMap::Flush()
{
	ERR_UNIMPLEMENTED();
	return ER_Failure;
}

errResult stoFileMemoryMap::Write(const void* buffer, size_t size)
{
	ERR_UNIMPLEMENTED(); // not tested for writing, do not use!

	const byte* pSrc = (const byte*)buffer;
	memcpy(m_pCurrentPos, pSrc, size);
	m_pCurrentPos += size;
	return ER_Success;
}

bool stoFileMemoryMap::IsValid() const
{
	return (m_hFile != INVALID_HANDLE_VALUE) && m_pStartPos && m_hMemoryMap;
}

int stoFileMemoryMap::Tell() const
{
    // note: possible pointer truncation in 64 bit mode
	return (int)(m_pCurrentPos - m_pStartPos);
}

#endif // #if CORE_SYSTEM_WINAPI
