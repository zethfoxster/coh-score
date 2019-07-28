/*****************************************************************************
	created:	2001/04/22
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/core/corArgFormatter.h"
#include <stdio.h>
#include <stdarg.h>

////////////
// Format the string using printf(), and return a pointer to the
// formatted output buffer.

const char *corArgFormatter::Format(const char* fmt, ...)
{
  // get varargs stuff
  va_list args;
  va_start(args, fmt);
  // format in buffer
  vsnprintf(m_buffer, bufsize, fmt, args);
  // end varargs
  va_end(args);
  // done
  return m_buffer;
}
