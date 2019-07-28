#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoCompressor.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef WIN32
#else
#define CoreZlib	// assumes that Zlib will use the system library on linux
#endif

const char *stoCompressor::kClearName = "Clear";
const char *stoCompressor::kZlibName  = "Zlib";
const char *stoCompressor::kBzip2Name = "Bzip2";
const char *stoCompressor::kUnchangedName = "Unchanged";

// not on Xenon or PS3
#if !CORE_SYSTEM_XENON&&!CORE_SYSTEM_PS3

#include "zlib.h"
#include "bzlib.h"

/**
 * gets the name of a compression scheme from an enum
 *
 *  @param scheme compression scheme to name
 *
 *  @return string name of compression scheme
**/
const char *stoCompressor::GetCompressionLabel(compressionScheme scheme)
{
	switch(scheme)
	{
	case kClear:
		return stoCompressor::kClearName;
		break;
	case kZlib:
		return stoCompressor::kZlibName;
		break;
	case kBzip2:
		return stoCompressor::kBzip2Name;
		break;
	case kUnchanged:
		return stoCompressor::kUnchangedName;

	}
	return NULL;
}

/**
 * gets the worst case size for a scheme for an input size
 *
 *  @param scheme compression scheme to use
 *  @param inSize size of the buffer that will be compressed
 *
 *  @return stoFile created
**/
int32 stoCompressor::GetWorstCase( compressionScheme scheme, int32 inSize)
{
	switch (scheme)
	{
	case kClear:
		return inSize;
		break;

	case kZlib:
		return 	(int)(inSize*1.02 + 24); // worst case for zlib
		break;

	case kBzip2:
		return 	(int)(inSize*1.01 + 600); // worst case for bzip2
		break;

    case kUnchanged:
        return inSize;
        break;

	}
	//printf("Invalid compression scheme!\n");
	return 0;
}


/**
 *  uncompress from a buffer to a buffer.
 *
 *  @param scheme compression scheme to use
 *  @param destBuf buffer to uncompress to
 *  @param size of dest buffer (out)
 *  @param srcBuf buffer to uncompress from
 *  @param srcSize size of src buffer
 *
 *  @return success or failure
**/
errResult stoCompressor::UnCompress(compressionScheme scheme, char *destBuf,
									int32 *destSize, char *srcBuf, int32 srcSize)
{
	int32 result;

	switch (scheme)
	{
	case kZlib:
		// uncompress with Zlib
		result = ::uncompress( 
				(Bytef*)destBuf,
				(uLongf*)destSize, 
				(Bytef*)srcBuf, 
				(uLongf)srcSize);

		switch (result)
		{
		case Z_OK:
			break;

		case Z_BUF_ERROR:
			printf("Error: uncompress buffer not large enough.\n");
			return ER_Failure;
			break;

		case Z_MEM_ERROR:
			printf("Error: Not enough memory.\n");
			return ER_Failure;
			break;

		case Z_DATA_ERROR:
			printf("Error: compressed data is bad!.\n");
			return ER_Failure;
			break;
		}
		break;

	case kBzip2:
		// uncompress with Bzip2
		result = BZ2_bzBuffToBuffDecompress( 
				(char *)destBuf,
				(unsigned int *)destSize, 
				(char *)srcBuf, 
				srcSize,
				0,0);

		if (result != BZ_OK)
		{
			return ER_Failure;
		}
		break;

	case kClear:
		// dont uncompress at all, copy data
		memcpy(destBuf, srcBuf, srcSize);
		*destSize =  srcSize;
		break;

    case kUnchanged:
        return ER_Failure;
	}
	return ER_Success;
}


/**
 *  compress from a buffer to a buffer.
 *
 *  @param scheme compression scheme to use
 *  @param destBuf buffer to uncompress to
 *  @param size of dest buffer (out)
 *  @param srcBuf buffer to uncompress from
 *  @param srcSize size of src buffer
 *
 *  @return success or failure
**/
errResult stoCompressor::Compress(compressionScheme scheme, char *destBuf,
								  int32 *destSize, char *srcBuf, int32 srcSize)
{
	int32 result;

	switch (scheme)
	{
	case kZlib:
		// compress with Zlib
		result = ::compress(
					(Bytef*)destBuf,
					(uLongf*)destSize, 
					(Bytef*)srcBuf, 
					(uLong)srcSize);
		if (result != Z_OK)
		{
			return ER_Failure;
		}
		break;

	case kBzip2:
		// compress with Bzip2
		result = BZ2_bzBuffToBuffCompress(
			(char *)destBuf,
			(unsigned int *)(destSize),
			(char *)srcBuf,
			srcSize,
			5,  // block size (determines how much memory is used (1-9) )
			0,  // verbosity
			30); // "workfactor" - this is the default;
		switch (result)
		{
		case BZ_OK:
			//success
			break;

		case BZ_OUTBUFF_FULL:
			printf("Bzip output full.\n");
			return ER_Failure;
			break;

		case BZ_MEM_ERROR:
			printf("Bzip memory error.\n");
			return ER_Failure;
			break;

		case BZ_PARAM_ERROR:
			printf("Bzip parm error.\n");
			return ER_Failure;
			break;
		}
		break;

	case kClear:
		// dont compress at all. just copy 
		memcpy(destBuf, srcBuf, srcSize);
		*destSize = srcSize;
		break;

    case kUnchanged:
        return ER_Failure;
	}
	return ER_Success;
}

#endif // CORE_SYSTEM_XENON

















