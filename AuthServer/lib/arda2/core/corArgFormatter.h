/*****************************************************************************
	created:	2001/04/22
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corArgFormatter
#define   INCLUDED_corArgFormatter
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Class wrapping up snprintf() in something that can be called safely
// from multiple threads.  Does not use thread-specific data, but a 
// local buffer, so it's not maximally efficient.

class corArgFormatter
{

private:

  enum { bufsize = 1024 };
  char* m_buffer;

public:

  corArgFormatter()  { m_buffer = new char[bufsize]; }
  ~corArgFormatter() { delete [] m_buffer; }
  const char * Format(const char* fmt, ...);

 private:

  // disabled
  corArgFormatter(const corArgFormatter&);
  corArgFormatter& operator=(const corArgFormatter&);
};

#endif // INCLUDED_corArgFormatter

