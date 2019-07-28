/*****************************************************************************
created:	2005/07/28
copyright:	2005, NCSoft. All Rights Reserved
author(s):	Philip Flesher

purpose:	extension of stoFileOSFile that encrypts/decrypts files

*****************************************************************************/

#include "arda2/core/corFirst.h"

// Note[tcg]: so if you don't have openssl then you can't build this file...
#if CORE_SYSTEM_XENON|CORE_SYSTEM_PS3
#else

#include "./stoFileCryptFile.h"
#include "arda2/storage/stoFileUtils.h"

stoFileCryptFile::stoFileCryptFile()
{
    this->m_key = 0;
    this->m_blockNum = 0;
    for (int i = 0; i < 8; ++i)
        this->m_initVector[i] = 0;
}

stoFileCryptFile::~stoFileCryptFile()
{
    if (this->m_key)
        delete this->m_key;
}

errResult stoFileCryptFile::Open(const char* filename, AccessMode mode, const unsigned char *key, uint keyLength)
{
    if (stoFileOSFile::Open(filename, mode) == ER_Failure)
        return ER_Failure;

    if (keyLength > stoFileCryptFile::MAX_KEY_LENGTH)
        return ER_Failure;

    if (this->m_key)
        delete this->m_key;
    this->m_key = new BF_KEY();
    BF_set_key(this->m_key, keyLength, key);

    for (int i = 0; i < 8; ++i)
        this->m_initVector[i] = 0;

    return ER_Success;
}

errResult stoFileCryptFile::Read(void* buffer, size_t size)
{
    unsigned char *tempBuffer = new unsigned char[size];

    errResult retVal = stoFileOSFile::Read(tempBuffer, size);
    if (retVal == ER_Success)
    {
        BF_cfb64_encrypt(tempBuffer, (unsigned char*) buffer, (long)size, this->m_key, this->m_initVector, &this->m_blockNum, BF_DECRYPT);
    }

    delete [] tempBuffer;
    return retVal;
}

errResult stoFileCryptFile::Write(const void* buffer, size_t size)
{
    unsigned char *tempBuffer = new unsigned char[size];

    BF_cfb64_encrypt((const unsigned char*)buffer, tempBuffer, (long)size, this->m_key, this->m_initVector, &this->m_blockNum, BF_ENCRYPT);
    errResult retVal = stoFileOSFile::Write(tempBuffer, size);

    delete [] tempBuffer;
    return retVal;
}

stoFileOSFile* stoFileCryptFile::OpenPlaintextOrEncryptedFile(const char *plaintextFilename, const char *encryptedFilename, 
                                                              const unsigned char *key, const uint keyLength, 
                                                              bool warnIfBothFilesExist, bool warnIfNeitherFileExists)
{
    stoFileOSFile *returnFile = 0;

    bool plaintextExists = stoFileUtils::FileExists(plaintextFilename);
    bool encryptedExists = stoFileUtils::FileExists(encryptedFilename);

    if (plaintextExists)
    {
        returnFile = new stoFileOSFile();
        returnFile->Open(plaintextFilename, stoFileOSFile::kAccessRead);

        if (encryptedExists && warnIfBothFilesExist)
        {
            ERR_REPORTV( ES_Warning, ("Both file %s and file %s exist. Using %s.", plaintextFilename, encryptedFilename, plaintextFilename));
        }
    }
    else if (encryptedExists)
    {
        returnFile = new stoFileCryptFile();
        static_cast<stoFileCryptFile*>(returnFile)->Open(encryptedFilename, stoFileOSFile::kAccessRead, key, keyLength);
    }
    else if (warnIfNeitherFileExists)
    {
        ERR_REPORTV( ES_Warning, ("Neither file %s nor file %s exists.", plaintextFilename, encryptedFilename, plaintextFilename));
    }

    return returnFile;
}


#endif // !(CORE_SYSTEM_XENON|CORE_SYSTEM_PS3)
