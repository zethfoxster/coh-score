/*****************************************************************************
	created:	2001/04/22
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_stoFile_h
#define   INCLUDED_stoFile_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corError.h"
#include "arda2/core/corStdString.h"


class stoFile  
{
public:

	enum SeekMode
	{
		kSeekBegin,
		kSeekCurrent,
		kSeekEnd
	};

	stoFile();
	virtual ~stoFile();

	// Seek - moves the file pointer to the specified offset
	virtual errResult	Seek( int offset, SeekMode mode = kSeekBegin ) = 0;

	// Rewind - typically same as Seek() to beginning
	virtual errResult	Rewind() = 0;

	// Flush - flush the buffer 
	virtual errResult	Flush() = 0;

	// Eof - check for end of file
	virtual bool		Eof() const = 0;

	// Read - read 'size' bytes into buffer from the file
	virtual errResult	Read(void* buffer, size_t size) = 0;

	// Write - writes 'size' byte from the buffer into the file
	virtual errResult	Write(const void* buffer, size_t size) = 0;

	virtual int			GetSize() const = 0;

	// IsValid - returns true when the file is valid (open)
	virtual bool		IsValid() const = 0;

	virtual bool		CanRead() const = 0;

	virtual bool		CanWrite() const = 0;

	virtual bool		CanSeek() const = 0;

	// Tell - returns the current offset into the file
	virtual int			Tell() const = 0;

    virtual void * AcquireMemoryBuffer(OUT uint &bufferSize);
    virtual void ReleaseMemoryBuffer();

	// Utility interface functions
	template <typename T>
	errResult			Write( const T& t )
	{ return Write(&t, sizeof(T)); }

	//template<>
	errResult			Write( const std::string& s )
	{ return WriteString(s); }

	template <typename T>
	errResult			Read( T& t)
	{ return Read(&t, sizeof(T)); }

	//template<>
	errResult			Read( std::string &s )
	{ return ReadString(s); }

	errResult			WriteString( const std::string &s );
	errResult			ReadString( std::string &s );

	// following functions are for text files only

	// WriteLine - writes a line to the file
	errResult			WriteLine( const std::string &s );

	// WriteLine - writes a line to the file
	errResult			WriteLine( const char *s );

	// ReadLine - read up until '\n' or EOF
	errResult			ReadLine( std::string &s );

	// Print-like output (line-delimited)
	errResult			FormatLine( const char* fmt, ... );
};


#endif // INCLUDED_stoFile_h
