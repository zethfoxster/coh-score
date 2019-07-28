
#include "package.h"

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#include <tchar.h>
#include <strsafe.h>
#include <assert.h>

#define PACKAGE_FILE_ALIGNMENT 4096
#define READ_BUFFER_SIZE 4096
#define ALIGN_FILE_OFFSET(x) (PACKAGE_FILE_ALIGNMENT * ((x + PACKAGE_FILE_ALIGNMENT - 1) / PACKAGE_FILE_ALIGNMENT))

static int package_inited = 0;
static HCRYPTPROV package_crypto = 0;
static U8 *zero_buffer = NULL;
static U8 *read_buffer = NULL;


// Utility function to report errors
// from http://msdn.microsoft.com/en-us/library/bb540534%28v=VS.85%29.aspx
void DisplayError(LPTSTR lpszFunction) 
// Routine Description:
// Retrieve and output the system error message for the last-error code
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, 
        NULL );

    lpDisplayBuf = 
        (LPVOID)LocalAlloc( LMEM_ZEROINIT, 
                            ( lstrlen((LPCTSTR)lpMsgBuf)
                              + lstrlen((LPCTSTR)lpszFunction)
                              + 40) // account for format string
                            * sizeof(TCHAR) );
    
    if (FAILED( StringCchPrintf((LPTSTR)lpDisplayBuf, 
                     LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                     TEXT("%s failed with error code %d as follows:\n%s"), 
                     lpszFunction, 
                     dw, 
                     lpMsgBuf)))
    {
        printf("FATAL ERROR: Unable to output error code.\n");
    }
    
    _tprintf(TEXT("  ==== ERROR ====\n%s  ==== END ERROR ====\n"), (LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

void Package_InitOnce(void)
{
	if (!package_inited)
	{
		// This code assumes 32-bit pointers
		assert(sizeof(const char *) == sizeof(U32));

		// Get handle to the crypto provider
		if (!CryptAcquireContext(&package_crypto,
			NULL,
			NULL,
			PROV_RSA_AES,			// PROV_RSA_FULL does not provide MD4
			CRYPT_VERIFYCONTEXT))
		{
			DisplayError("CryptAcquireContext()");
		}

		if (!zero_buffer)
		{
			zero_buffer = malloc(PACKAGE_FILE_ALIGNMENT);
			memset(zero_buffer, 0, PACKAGE_FILE_ALIGNMENT);
		}

		if (!read_buffer)
		{
			read_buffer = malloc(READ_BUFFER_SIZE);
		}
	}

}

void Package_Shutdown(void)
{
	if (package_inited)
	{
		CryptReleaseContext(package_crypto, 0);

		package_inited = 0;

		free(zero_buffer);
		zero_buffer = NULL;

		free(read_buffer);
		read_buffer = NULL;
	}
}



// Returns 1 on success, 0 on failure
static int calculateHash(const U8 *pData, size_t dataSize, U8 hash[] )
{
	// from http://msdn.microsoft.com/en-us/library/aa382380%28v=VS.85%29.aspx
	BOOL bResult = FALSE;
	HCRYPTHASH hHash = 0;
	DWORD hashLen = PACKAGE_HASH_SIZE;

	// Went with MD4 because it is cheaper to calculate than MD5.
	// If this is changed, then make sure PACKAGE_VERSION is changed and
	// PACKAGE_HASH_SIZE is still valid.
	if (!CryptCreateHash(package_crypto, CALG_MD4, 0, 0, &hHash))
	{
		DisplayError("CryptCreateHash() in calculateHash()");
		return 0;
	}

	if (!CryptHashData(hHash, pData, dataSize, 0))
	{
		DisplayError("CryptHashData() in calculateHash()");
		CryptDestroyHash(hHash);
		return 0;
	}

	if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0))
	{
		DisplayError("CryptGetHashParam() in calculateHash()");
		CryptDestroyHash(hHash);
		return 0;
	}

	CryptDestroyHash(hHash);

	return 1;
}

// populates the file info size and hash
// Returns 1 on success, 0 on failure
static int PopulateFileSizeAndHash(PackageFileInfo *fileInfo)
{
    HANDLE hFile = NULL;
	BOOL bResult = FALSE;
	HCRYPTHASH hHash = 0;
	DWORD hashLen = PACKAGE_HASH_SIZE;
	U32 bytesRead = 0;

	assert(fileInfo->filename);
	fileInfo->fileSize = 0;

    hFile = CreateFile(fileInfo->filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
		DisplayError("CreateFile() in PopulateFileSizeAndHash()");
        return 0;
    }

	if (!CryptCreateHash(package_crypto, CALG_MD4, 0, 0, &hHash))
	{
		DisplayError("CryptCreateHash() in PopulateFileSizeAndHash()");
		CloseHandle(hFile);
		return 0;
	}

    while (bResult = ReadFile(hFile, read_buffer, READ_BUFFER_SIZE, &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            break;
        }

		fileInfo->fileSize += bytesRead;

		if (!CryptHashData(hHash, read_buffer, bytesRead, 0))
		{
			DisplayError("CryptHashData() in PopulateFileSizeAndHash()");
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return 0;
		}
	}

	if (!CryptGetHashParam(hHash, HP_HASHVAL, fileInfo->fileHash, &hashLen, 0))
	{
		DisplayError("CryptGetHashParam() in PopulateFileSizeAndHash()");
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return 0;
	}

	CryptDestroyHash(hHash);
	CloseHandle(hFile);

	return 1;
}

// Takes a list of files and makes a PackageHeader.  Marks all files available
PackageHeader *Package_CreatePackageHeader(const char *packageName, const char **filenames, int numFiles)
{
	int i;
	int filename_memsize = 0;
	int availability_memsize = 0;
	int total_header_size = 0;

	PackageHeader *header = NULL;
	PackageFileInfo *fileInfoArray = NULL;
	char *filename_memory = NULL;
	U8 *availabilityArray = NULL;

	PackageFileInfo *currentFileInfo = NULL;
	char *current_filename = NULL;
	U32 currentFileOffset = 0;
	U32 filename_len;


	assert(numFiles > 0);
	if (numFiles < 1)
	{
		printf("Need at least one file for the package: numFiles = %d\n", numFiles);
		return NULL;
	}

	assert(filenames);
	if (!filenames)
	{
		printf("Filename array is NULL\n");
		return NULL;
	}

	// First, determine the amount of memory needed for the filenames
	filename_memsize = strlen(packageName) + 1;
	for (i = 0; i < numFiles; i++)
	{
		DWORD fileAttribs;

		if (!filenames[i])
		{
			printf("filename at index %d is NULL\n", i);
			return NULL;
		}

		fileAttribs = GetFileAttributes(filenames[i]);

		if (fileAttribs == INVALID_FILE_ATTRIBUTES)
		{
			DisplayError("GetFileAttributes() in Package_CreatePackageHeader()");
			printf("Failed to get file attributes for %s\n", filenames[i]);
			return NULL;
		}

		if (fileAttribs & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
		{
			printf("File %s is not a normal file: file attributes = %d\n", filenames[i], fileAttribs);
			return NULL;
		}

		filename_memsize += strlen(filenames[i]) + 1;
	}

	availability_memsize = numFiles;

	// Calculate total header size
	total_header_size = sizeof(PackageHeader) +
						numFiles * sizeof(PackageFileInfo) +
						filename_memsize +
						availability_memsize;

	// Allocate memory
	header = (PackageHeader*) malloc(total_header_size);
	fileInfoArray = (PackageFileInfo*) ((U8*) header + sizeof(PackageHeader));
	filename_memory = (char*) ((U8*) fileInfoArray + numFiles * sizeof(PackageFileInfo));
	availabilityArray = (U8*) filename_memory + filename_memsize;

	// Populate availability
	memset(availabilityArray, PackageFile_Available, availability_memsize);

	// Populate file names and PackageFileInfo's
	current_filename = filename_memory;
	currentFileInfo = fileInfoArray;
	currentFileOffset = ALIGN_FILE_OFFSET(total_header_size);

	filename_len = strlen(packageName) + 1;
	memcpy(current_filename, packageName, filename_len);
	header->filename = current_filename;
	current_filename += filename_len;

	for (i = 0; i < numFiles; i++)
	{
		// Populate file name
		filename_len = strlen(filenames[i]) + 1;
		memcpy(current_filename, filenames[i], filename_len);

		// Populate PackageFileInfo
		currentFileInfo->filename = current_filename;
		if (PopulateFileSizeAndHash(currentFileInfo) == 0)
		{
			free(header);
			return NULL;
		}

		currentFileInfo->fileOffset = currentFileOffset;

		// Update file offset
		currentFileOffset += ALIGN_FILE_OFFSET(currentFileInfo->fileSize);

		// Sanity Checks
		assert(currentFileInfo->fileSize > 0);
		assert(currentFileInfo->fileOffset > 0);

		// Advance to next filename and PackageFileInfo
		current_filename += filename_len;
		currentFileInfo++;
	}
	
	// Populate package header
	header->headerID = PACKAGE_MAGIC_ID;
	header->version = PACKAGE_VERSION;
	header->headerDataSize = total_header_size;
	header->numFiles = numFiles;
	header->fileInfos = fileInfoArray;
	header->filenames = filename_memory;
	header->availability = availabilityArray;
	if (calculateHash((U8*) fileInfoArray, numFiles * sizeof(PackageFileInfo) + filename_memsize, header->HeaderHash) == 0)
	{
		free(header);
		return NULL;
	}

	return header;
}

// Writes availability information out to disk
int Package_FlushAvailability(PackageHeader *header)
{
	HANDLE packageFile;
	U32 availabilityOffset;
	U32 availability_memsize;
	U32 bytesWritten = 0;

	assert(header);
	assert(header->fileInfos);
	assert(header->filenames);

	// Open the package file for writing
    packageFile = CreateFile(header->filename,
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,          // open an existing file
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (packageFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() in Package_FlushAvailability()");
        return 0;
    }

	// Set current file position
	availabilityOffset = (U32) header->availability - (U32) header;
	if (SetFilePointer(packageFile, availabilityOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		DisplayError("SetFilePointer() in Package_FlushAvailability()");
		CloseHandle(packageFile);
        return 0;
    }

	// Write availability to the package file
	availability_memsize = header->numFiles;
	if (WriteFile(packageFile, header->availability, availability_memsize, &bytesWritten, NULL) == FALSE)
	{
		DisplayError("WriteFile() in Package_FlushAvailability()");
		CloseHandle(packageFile);
		return 0;
	}

	// close package file
	CloseHandle(packageFile);

	return 1;
}

// Mark a file as unavailable, but do not modify its current contents.
// Useful when creating a sparse package file
int Package_MarkFileZeros(PackageHeader *header, U32 fileIndex)
{
	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->availability);

	header->availability[fileIndex] = PackageFile_Zeros;

	return 1;
}

// Mark a file as unavailable, but do not modify its current contents.
// Useful when the file is out-of-date and needs to be updated
int Package_MarkFileUnavailable(PackageHeader *header, U32 fileIndex)
{
	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->availability);

	header->availability[fileIndex] = PackageFile_NotAvailable;

	return 1;
}

// Mark a file as available, but do not modify its current contents.
// Useful when the file has been updated
static int Package_MarkFileAvilable(PackageHeader *header, U32 fileIndex)
{
	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->availability);

	header->availability[fileIndex] = PackageFile_Available;

	return 1;
}

PackageFileAvailability Package_GetFileAvailability(const PackageHeader *header, U32 fileIndex)
{
	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->availability);

	return header->availability[fileIndex];
}

// Assumes the file position of the package file is at the desired location
static int WriteFileToPackage(const char *source_file, U32 fileLen, HANDLE packageFile, U8 *ptrToHash)
{
    HANDLE hFile = NULL;
	BOOL bResult = FALSE;
	U32 bytesRead = 0;
	U32 totalBytesRead = 0;
	HCRYPTHASH hHash = 0;
	DWORD hashLen = PACKAGE_HASH_SIZE;
	int rc = 1;

	assert(source_file);
	assert(read_buffer);

	if (ptrToHash && !CryptCreateHash(package_crypto, CALG_MD4, 0, 0, &hHash))
	{
		DisplayError("CryptCreateHash() in WriteFileToPackage()");
		return 0;
	}

    hFile = CreateFile(source_file,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
		DisplayError("CreateFile() in WriteFileToPackage()");
        rc = 0;
    }

    while (rc && (bResult = ReadFile(hFile, read_buffer, READ_BUFFER_SIZE, &bytesRead, NULL)))
    {
		U32 bytesWritten = 0;

		if (FALSE == bResult)
		{
			DisplayError(TEXT("ReadFile() in WriteFileToPackage()"));
			rc = 0;
			return 0;
		}

        if (bytesRead == 0)
        {
            break;
        }

		bResult = WriteFile(packageFile, read_buffer, bytesRead, &bytesWritten, NULL);

		if (FALSE == bResult || bytesWritten != bytesRead)
		{
			DisplayError(TEXT("WriteFile() in WriteFileToPackage()"));
			rc = 0;
			break;
		}

		if (ptrToHash && !CryptHashData(hHash, read_buffer, bytesRead, 0))
		{
			DisplayError("CryptHashData() in WriteFileToPackage()");
			rc = 0;
			break;
		}

		totalBytesRead += bytesWritten;
	}

	CloseHandle(hFile);

	if (ptrToHash)
	{
		if (rc && !CryptGetHashParam(hHash, HP_HASHVAL, ptrToHash, &hashLen, 0))
		{
			DisplayError("CryptGetHashParam() in WriteFileToPackage()");
			rc = 0;
		}

		CryptDestroyHash(hHash);
	}

	if (rc && totalBytesRead != fileLen)
	{
		rc = 0;
		assert(totalBytesRead == fileLen);
	}

	return rc;
}

// Assumes the file position of the package file is at the desired location
static int PadWithZeros(HANDLE packageFile, U32 currentOffset, U32 minZerosToWrite)
{
	U32 desiredEnd = ALIGN_FILE_OFFSET(currentOffset + minZerosToWrite);
	U32 zerosToWrite = desiredEnd - currentOffset;

	assert(zero_buffer);

	while(zerosToWrite > 0)
	{
		U32 bytesWritten = 0;
		U32 bytesToWrite = (zerosToWrite > PACKAGE_FILE_ALIGNMENT) ? PACKAGE_FILE_ALIGNMENT : zerosToWrite;
		BOOL bErrorFlag = FALSE;

		bErrorFlag = WriteFile( 
						packageFile,
						zero_buffer,
						bytesToWrite,
						&bytesWritten,
						NULL);

		if (FALSE == bErrorFlag)
		{
			DisplayError(TEXT("WriteFile() in PadWithZeros()"));
			return 0;
		}

		zerosToWrite -= bytesWritten;
	}

	return 1;
}

// This will create the package file on disk and for available files, populate them with
// files from the local directory.  For unavailable files, their data will be all zeros.
int Package_Create(const PackageHeader *header)
{
	U32 i;
	HANDLE packageFile;
	PackageHeader *outHeader;
	PackageFileInfo *outFileInfos;
	BOOL bResult = FALSE;
	U32	bytesWritten;

	assert(header);
	assert(header->filename);
	assert(header->headerID == PACKAGE_MAGIC_ID);
	assert(header->version == PACKAGE_VERSION);
	assert(header->numFiles > 0);
	assert(header->headerDataSize > sizeof(PackageHeader) + sizeof(PackageFileInfo) * header->numFiles);
	assert(header->fileInfos);
	assert(header->filenames);
	assert(header->availability);

	// Create the file for writing
    packageFile = CreateFile(header->filename,
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_NEW,             // create new file only
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template
 
    if (packageFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() in Package_Create()");
        return 0;
    }

	// Make a copy of the header data so we can fixup the pointers to offsets to write to disk
	outHeader = malloc(header->headerDataSize);
	outFileInfos = (PackageFileInfo*) ((U8*) outHeader + sizeof(PackageHeader));

	memcpy(outHeader, header, header->headerDataSize);

	// Change pointers to offsets
	(U32) (outHeader->filename) = (U32) header->filename - (U32) header->filenames;
	(U32) (outHeader->fileInfos) = (U32) header->fileInfos - (U32) header;
	(U32) (outHeader->filenames) = (U32) header->filenames - (U32) header;
	(U32) (outHeader->availability) = (U32) header->availability- (U32) header;

	for (i = 0; i < header->numFiles; i++)
	{
		(U32) (outFileInfos[i].filename) = (U32) header->fileInfos[i].filename - (U32) header->filenames;
	}

	// Write the header information
	bResult = WriteFile(packageFile, outHeader, header->headerDataSize, &bytesWritten, NULL);

	if (FALSE == bResult)
	{
		DisplayError(TEXT("WriteFile() in Package_Create()"));
		free(outHeader);
		CloseHandle(packageFile);
		return 0;
	}

	// Pad out the header information to the first PackageFileInfo
	if (PadWithZeros(packageFile, bytesWritten, 0) != 1)
	{
		free(outHeader);
		CloseHandle(packageFile);
		return 0;
	}

	bytesWritten = ALIGN_FILE_OFFSET(bytesWritten);

	// For each file
	for (i = 0; i < header->numFiles; i++)
	{
		assert(header->fileInfos[i].fileOffset == bytesWritten);

		if (Package_GetFileAvailability(header, i) == PackageFile_Available)
		{
			// Write file to package
			// TODO: Check each file hash?  Presumably, Package_CreatePackageHeader() was just called
			// which just calculated the file hash from the same file.
			if (WriteFileToPackage(header->fileInfos[i].filename, header->fileInfos[i].fileSize, packageFile, NULL) != 1)
			{
				free(outHeader);
				CloseHandle(packageFile);
				return 0;
			}

			bytesWritten += header->fileInfos[i].fileSize;

			// Pad the package file
			if (PadWithZeros(packageFile, bytesWritten, 0) != 1)
			{
				free(outHeader);
				CloseHandle(packageFile);
				return 0;
			}

			bytesWritten = ALIGN_FILE_OFFSET(bytesWritten);
		}
		else
		{
			assert(Package_GetFileAvailability(header, i) == PackageFile_Zeros);

			// Write all 0's
			if (PadWithZeros(packageFile, bytesWritten, header->fileInfos[i].fileSize) != 1)
			{
				free(outHeader);
				CloseHandle(packageFile);
				return 0;
			}


			bytesWritten += ALIGN_FILE_OFFSET(header->fileInfos[i].fileSize);
		}
	}

	// Close the file
	CloseHandle(packageFile);
	return 1;
}

// Extracts a file to the current working directory
int Package_UnpackFile(PackageHeader *header, U32 fileIndex)
{
	HANDLE packageFile;
	HANDLE destFile;
	U32 remaining_bytes;

	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->fileInfos);
	assert(header->filenames);
	assert(header->filename);

	if (Package_GetFileAvailability(header, fileIndex) != PackageFile_Available)
	{
		return 0;
	}

	// Open the package file for reading
    packageFile = CreateFile(header->filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (packageFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() in Package_UnpackFile()");
        return 0;
    }

	// Set current file position
	if (SetFilePointer(packageFile, header->fileInfos[fileIndex].fileOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		DisplayError("SetFilePointer() in Package_UpdateFile()");
		CloseHandle(packageFile);
        return 0;
    }

	// Open destination file
    destFile = CreateFile(header->fileInfos[fileIndex].filename,
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       CREATE_NEW,             // create new file only
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template
 

    if (destFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() for destination in Package_UnpackFile()");
        return 0;
    }

	// Loop over the file contents in the package
	remaining_bytes = header->fileInfos[fileIndex].fileSize;

    while (remaining_bytes)
	{
		U32 bytes_to_read = (remaining_bytes > READ_BUFFER_SIZE) ? READ_BUFFER_SIZE : remaining_bytes;
		U32 bytesRead;
		U32 bytesWritten;
		BOOL bResult = FALSE;

		// Read from package file
		bResult = ReadFile(packageFile, read_buffer, bytes_to_read, &bytesRead, NULL);
        if (bResult == FALSE)
        {
			DisplayError(TEXT("ReadFile() in Package_UnpackFile()"));
			CloseHandle(destFile);
			CloseHandle(packageFile);
            return 0;
        }

		// Write to destination file
		bResult = WriteFile(destFile, read_buffer, bytesRead, &bytesWritten, NULL);

        if (bResult == FALSE)
		{
			DisplayError(TEXT("WriteFile() in Package_UnpackFile()"));
			CloseHandle(destFile);
			CloseHandle(packageFile);
			return 0;
		}

		remaining_bytes -= bytesWritten;
	}

	// close destination file
	CloseHandle(destFile);

	// close package file
	CloseHandle(packageFile);

	return 1;
}

// Updates a file in the package from the current working directory
int Package_UpdateFile(PackageHeader *header, U32 fileIndex)
{
	HANDLE packageFile;
	U32 availabilityOffset;
	U32 availability_memsize;
	U32 bytesWritten;
	U8  fileHash[PACKAGE_HASH_SIZE];

	assert(header);
	assert(fileIndex < header->numFiles);
	assert(header->fileInfos);
	assert(header->filenames);

	assert(Package_GetFileAvailability(header, fileIndex) != PackageFile_Available);

	// Open the package file for writing
    packageFile = CreateFile(header->filename,
                       GENERIC_WRITE,          // open for writing
                       0,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,          // open an existing file
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (packageFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() in Package_UpdateFile()");
        return 0;
    }

	// Set current file position
	if (SetFilePointer(packageFile, header->fileInfos[fileIndex].fileOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		DisplayError("SetFilePointer() in Package_UpdateFile()");
		CloseHandle(packageFile);
        return 0;
    }

	// Write source file to the package file
	if (WriteFileToPackage(header->fileInfos[fileIndex].filename, header->fileInfos[fileIndex].fileSize, packageFile, fileHash) != 1)
	{
		CloseHandle(packageFile);
		return 0;
	}

	// Pad the package file
	if (PadWithZeros(packageFile, header->fileInfos[fileIndex].fileSize, 0) != 1)
	{
		CloseHandle(packageFile);
		return 0;
	}

	// See if the file hash matches what we are expecting
	if (memcmp(fileHash, header->fileInfos[fileIndex].fileHash, PACKAGE_HASH_SIZE) == 0)
	{
		// Mark file as available
		if (Package_MarkFileAvilable(header, fileIndex) != 1)
		{
			CloseHandle(packageFile);
			return 0;
		}

		// Set current file position for availability
		availabilityOffset = (U32) header->availability- (U32) header;
		if (SetFilePointer(packageFile, availabilityOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			DisplayError("SetFilePointer() in Package_FlushAvailability()");
			CloseHandle(packageFile);
			return 0;
		}

		// Write availability to the package file
		availability_memsize = header->numFiles;
		if (WriteFile(packageFile, header->availability, availability_memsize, &bytesWritten, NULL) == FALSE)
		{
			DisplayError("WriteFile() in Package_FlushAvailability()");
			CloseHandle(packageFile);
			return 0;
		}
	}
	else
	{
		printf("File hash from '%s' did not match expected file hash\n", header->fileInfos[fileIndex].filename);
		CloseHandle(packageFile);
		return 0;
	}

	// close package file
	CloseHandle(packageFile);

	return 1;
}


// Open an existing package and read its header and file information
PackageHeader *Package_ReadHeader(const char *package_filename)
{
	HANDLE packageFile;
	BOOL bResult = FALSE;
	PackageHeader *tmpHeader;
	PackageHeader *header;
	U32 i;
	U32 bytes_to_read;
	U32 bytes_read;

	assert(read_buffer);
	assert(sizeof(PackageHeader) < READ_BUFFER_SIZE);

	tmpHeader = (PackageHeader *) read_buffer;

	// Open the package file for reading
    packageFile = CreateFile(package_filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (packageFile == INVALID_HANDLE_VALUE)
    {
		DisplayError("CreateFile() in Package_ReadHeader()");
        return 0;
    }

	// Read just the header
	bytes_to_read = sizeof(PackageHeader);
    bResult = ReadFile(packageFile, tmpHeader, bytes_to_read, &bytes_read, NULL);
	
	if (bResult != TRUE || bytes_read != bytes_to_read)
	{
		DisplayError("ReadFile() in Package_ReadHeader()");
		CloseHandle(packageFile);
        return 0;
	}

	// Sanity checks
	if (tmpHeader->headerID != PACKAGE_MAGIC_ID)
	{
		printf("package magic ID is not correct\n");
		CloseHandle(packageFile);
        return 0;
	}

	// TODO: Handle old package versions when we make newer versions
	if (tmpHeader->version != PACKAGE_VERSION)
	{
		printf("package version is not correct\n");
		CloseHandle(packageFile);
        return 0;
	}

	// Allocate memory
	header = (PackageHeader*) malloc(tmpHeader->headerDataSize);

	// Copy what we've already read
	memcpy(header, tmpHeader, sizeof(PackageHeader));

	// Read rest of the header
	bytes_to_read = tmpHeader->headerDataSize - sizeof(PackageHeader);
    bResult = ReadFile(packageFile, header + 1, bytes_to_read, &bytes_read, NULL);
	
	if (bResult != TRUE || bytes_read != bytes_to_read)
	{
		DisplayError("ReadFile() in Package_ReadHeader()");
		CloseHandle(packageFile);
        return 0;
	}

	CloseHandle(packageFile);

	// Fixup pointers
	header->fileInfos =    (PackageFileInfo*) ((U8*) header + (U32) header->fileInfos);
	header->filenames =    (char*)            ((U8*) header + (U32) header->filenames);
	header->availability =                    ((U8*) header + (U32) header->availability);

	header->filename =         (char*)            ((U8*) header->filenames + (U32) header->filename);

	for (i = 0; i < header->numFiles; i++)
	{
		header->fileInfos[i].filename = header->filenames + (U32) header->fileInfos[i].filename;
	}

	return header;
}

