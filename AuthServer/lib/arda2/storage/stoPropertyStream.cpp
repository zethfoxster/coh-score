
#include "arda2/core/corFirst.h"
#include "arda2/storage/stoPropertyStream.h"


/****************** output file stream implementation ***************/

stoStreamOutputFile::stoStreamOutputFile() :
 m_file(NULL)
{
}

stoStreamOutputFile::~stoStreamOutputFile()
{
}

void stoStreamOutputFile::Init(stoFile *file)
{
    m_file = file;
}

errResult stoStreamOutputFile::Write( IN const char *data, IN uint size)
{
    if (!m_file)
        return ER_Failure;

    return m_file->Write(data, size);
}


/****************** input file stream implementation ***************/

stoStreamInputFile::stoStreamInputFile() : 
  m_file(NULL)
{
}

stoStreamInputFile::~stoStreamInputFile()
{
}

void stoStreamInputFile::Init(stoFile *file)
{
    m_file = file;
}

errResult stoStreamInputFile::Read( IN char *data, IN uint maxSize, OUT uint &bytesRead)
{
    if (!m_file)
        return ER_Failure;

    errResult ret = ER_Success;

    int previous = m_file->Tell();
    if (!ISOK(m_file->Read(data, maxSize)) )
    {
        int after = m_file->Tell();
        bytesRead = after - previous;
        ret = ER_Failure;
    }
    else
    {
        bytesRead = maxSize;
    }
    return ret;    
}
