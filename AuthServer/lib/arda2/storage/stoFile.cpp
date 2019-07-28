/*****************************************************************************
	created:	2001/04/23
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoFile.h"
#include <stdarg.h>
#include <stdio.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

stoFile::stoFile()
{
}

stoFile::~stoFile()
{
}


errResult stoFile::WriteString( const string &s )
{
	// write the string, including the trailing \0
	return Write(s.c_str(), s.length() + 1);
}


errResult stoFile::ReadString( string &s )
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

void *stoFile::AcquireMemoryBuffer(OUT uint &bufferSize)
{
    ERR_REPORT(ES_Error, ("Acquiring stoFile memory buffer in base class?!"));
    bufferSize = 0;
    return NULL;
}

void stoFile::ReleaseMemoryBuffer()
{
    ERR_REPORT(ES_Error, ("Releasing stoFile memory buffer in base class?!"));
}


// WriteLine - writes a line to the file
errResult stoFile::WriteLine( const string &s )
{
	errResult r = Write(s.c_str(), s.length());
	if ( !ISOK(r) )
		return r;

	return Write("\r\n", 2);
}


// WriteLine - writes a line to the file
errResult stoFile::WriteLine( const char *sz )
{
	errResult r = Write(sz, strlen(sz));
	if ( !ISOK(r) )
		return r;

	return Write("\r\n", 2);
}


// ReadLine - read up until '\n', '\r\n', or '\r' or EOF
errResult stoFile::ReadLine( string &s )
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
			Seek(-1, kSeekCurrent);
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

// Print-like output (line-delimited)
errResult stoFile::FormatLine( const char *fmt, ... )
{
	char buffer[1024];
	// get varargs stuff
	va_list args;
	va_start(args, fmt);
	// format in buffer
	int nLen = vsnprintf(buffer, sizeof(buffer), fmt, args);
	// end varargs
	va_end(args);
	// done
	
	errResult r = Write(buffer, nLen);
	if ( !ISOK(r) )
		return r;
	
	return Write("\r\n", 2);
}
