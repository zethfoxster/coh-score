/*****************************************************************************
created:	2005/07/28
copyright:	2005, NCSoft. All Rights Reserved
author(s):	Philip Flesher

purpose:	extension of stoFileOSFile that encrypts/decrypts files

*****************************************************************************/

// Note[tcg]: so if you don't have openssl then you can't build this file...
#if CORE_SYSTEM_XENON|CORE_SYSTEM_PS3
#define _INCLUDE_STOFILECRYPTFILE_H_
#endif

#ifndef _INCLUDE_STOFILECRYPTFILE_H_
#define _INCLUDE_STOFILECRYPTFILE_H_

#include "arda2/storage/stoFileOSFile.h"


// Note: The use of this class requires the linking of the openssl
// library libeay32.lib in NIH/openssl/lib.
#include "openssl/blowfish.h"

class stoFileCryptFile : public stoFileOSFile
{

public :

    explicit stoFileCryptFile();
    virtual ~stoFileCryptFile();

    errResult Open(const char *filename, AccessMode mode, const unsigned char *key, uint keyLength);

    // Seek - moves the file pointer to the specified offset
    virtual errResult Seek(int, SeekMode)
    {
        return ER_Failure;
    }

    // Read - read 'size' bytes into buffer from the file
    virtual errResult Read(void* buffer, size_t size);

    // Write - writes 'size' byte from the buffer into the file
    virtual errResult Write(const void* buffer, size_t size);

    virtual bool CanSeek() const
    {
        return false;
    }

    // Creates a new stoFileOSFile or stoFileCryptFile and attempts to open it.
    // Both the plaintext and encrypted filenames will be checked for existence
    // (unless a null value is passed in for one or both of the filenames), and if
    // both exist, a warning will be logged, with the plaintext version of the file
    // being opened and returned to the caller.  Similarly, if neither file exists,
    // a warning will be logged.  Each type of warning can be disabled independently
    // of the other.  Note that the user is responsible for deallocation of the
    // stoFileOSFile returned by this function.
    static stoFileOSFile* OpenPlaintextOrEncryptedFile(
        const char *plaintextFilename, const char *encryptedFilename, 
        const unsigned char *key, const uint keyLength, 
        bool warnIfBothFilesExist = true, bool warnIfNeitherFileExists = true);

private :

    static const uint MAX_KEY_LENGTH = 16;

    BF_KEY *m_key;
    unsigned char m_initVector[8];
    int m_blockNum;

};

#endif // #ifndef _INCLUDE_STOFILECRYPTFILE_H_
