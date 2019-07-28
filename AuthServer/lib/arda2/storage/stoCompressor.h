/**
 *  stoCompressor is a place to put compression schemes that are
 *  used by the pack file system. This isolates the application code
 *  from the compression libraries.
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef INCLUDED_CORE_COMPRESSOR_H
#define INCLUDED_CORE_COMPRESSOR_H

#include "arda2/core/corError.h"

class stoCompressor
{
public:
	enum compressionScheme
	{
		kClear,
		kZlib,
		kBzip2,
        kUnchanged // not an actual scheme - specifies to NOT change the existing compression of a file
	};
	
	static const char *kClearName;
	static const char *kZlibName;
	static const char *kBzip2Name;
    static const char *kUnchangedName;

	// gets the worst case size for a scheme for an input size
	static int32 GetWorstCase( compressionScheme scheme, int32 inSize);

	// gets a string name of the compression
	static const char *GetCompressionLabel(compressionScheme scheme);

	// un-compresses a buffer to a buffer
	static errResult UnCompress(
						compressionScheme  scheme,
						char			   *destBuf,	// destinationn buffer
						int32              *destSize,	// size of dest buffer (out)
						char			  *srcBuf,		// source buffer
						int32              srcSize);	// size of source buffer (in)

	// compresses a buffer to a buffer
	static errResult Compress(
						compressionScheme  scheme,
						char			   *destBuf,	// destinationn buffer
						int32              *destSize,	// size of dest buffer (out)
						char			   *srcBuf,		// source buffer
						int32              srcSize);	// size of source buffer (in)
};


#endif // INCLUDED_CORE_COMPRESSOR_H
