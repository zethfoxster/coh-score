/* sha.h: Defines the global variables and types for the SHA (Secure Hash Algorithm) library.
* 
* @author: Chris Cowden, ccowden@ncsoft.com
*
* created: 8/23/2006
*
* @file
*
* Functions:
*/

#ifndef _SHA_H_
#define _SHA_H_

#include "arda2/core/corFirst.h"
#include "arda2/core/corStdString.h"

using namespace std;

namespace cryptLib
{
	#define w_32_bits 32	// SHA-1, SHA-256 uses w=32, i.e. 32-bit "blocks"
	#define w_64_bits 64	// SHA-512 uses w=64, i.e. 64-bit "blocks"

	// the 512-bit hash (message digest)
	struct digest512
	{
		uint64 _[8];
	  string ToString()
	  {
	    char buf[129];

#ifdef WIN32

	    snprintf(buf,129,"%16.16I64x%16.16I64x%16.16I64x%16.16I64x%16.16I64x%16.16I64x%16.16I64x%16.16I64x\0",_[0],_[1],_[2],_[3],_[4],_[5],_[6],_[7]);

#else // LINUX

	    snprintf(buf,129,"%16.16llx%16.16llx%16.16llx%16.16llx%16.16llx%16.16llx%16.16llx%16.16llx",_[0],_[1],_[2],_[3],_[4],_[5],_[6],_[7]);

#endif

	    return string(buf);
	  }
	};

} // namespace cryptLib

#endif // _SHA_H_
