#pragma once
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include "stdtypes.h"

#define PACKAGE_MAGIC_ID 0x60164063
#define PACKAGE_HASH_SIZE 16	// 128-bit, matches MD4 and MD5 hash size
#define PACKAGE_VERSION 1

/*
 * PackageHeader
 * Array of PackageFileInfo's
 * Array of filename strings
 * Array of availability bytes
 * First file data (on 4K offset alignment)
 * Second file data (on 4K offset alignment)
 * etc.
 */

typedef struct PackageFileInfo PackageFileInfo;

typedef enum
{
	PackageFile_Zeros			= 0,
	PackageFile_NotAvailable	= 1,	// old data or data with bad hash
	PackageFile_Available		= 2,
} PackageFileAvailability;

// The HeaderHash in the header can be used to quickly determine if the file information in the package is up-to-date
typedef struct
{
	U32					headerID;			// Should be PACKAGE_MAGIC_ID
	U32					version;			// Should be PACKAGE_VERSION
	U32					headerDataSize;		// Includes PackageFileInfo's, filenames, availability
	const char 			*filename;			// On disk, this is the offset within the filename area of the filename 
	U32					numFiles;
	U8					HeaderHash[PACKAGE_HASH_SIZE];		// Hash of all of the PackageFileInfo's and filename strings
	PackageFileInfo		*fileInfos;			// On disk, this is the offset from the start of the file to the first PackageFileInfo
	const char			*filenames;			// On disk, this is the offset from the start of the file to the first filename
	U8					*availability;		// On disk, this is the offset from the start of the file to the first availability byte
} PackageHeader;

typedef struct PackageFileInfo
{
	U32					fileSize;
	U32					fileOffset;			// Offset from the beginning of the package file (should be 4k aligned)
	const char			*filename;			// On disk, this is the offset within the filename area of the filename
	U8					fileHash[PACKAGE_HASH_SIZE];
} PackageFileInfo;



void Package_InitOnce(void);
void Package_Shutdown(void);

// Takes a list of files and makes a PackageHeader.  Marks all files available
PackageHeader *Package_CreatePackageHeader(const char *packageName, const char **filenames, int numFiles);

// Mark a file as zeros.  Does not update the package file
// Useful when creating a sparse package file
int Package_MarkFileZeros(PackageHeader *header, U32 fileIndex);

// Mark a file as unavailable.  Does not update the package file
// Useful when the file is out-of-date and needs to be updated
int Package_MarkFileUnavailable(PackageHeader *header, U32 fileIndex);

// Writes availability information out to disk
int Package_FlushAvailability(PackageHeader *header);

// This will create the package file on disk and for available files, populate them with
// files from the local directory.  For unavailable files, their data will be all zeros.
int Package_Create(const PackageHeader *header);

// Extracts a file to the current working directory
int Package_UnpackFile(PackageHeader *header, U32 fileIndex);

// Updates a file in the package from the current working directory
int Package_UpdateFile(PackageHeader *header, U32 fileIndex);


// Open an existing package and read its header and file information
PackageHeader *Package_ReadHeader(const char *package_filename);

PackageFileAvailability Package_GetFileAvailability(const PackageHeader *header, U32 fileIndex);

int Package_LoadFile(PackageHeader *header, U32 fileIndex, void *location);

#endif // _PACKAGE_H_
