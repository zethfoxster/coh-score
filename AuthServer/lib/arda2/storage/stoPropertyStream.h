/**
 *  property system output stream
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __STO_STREAM_H__
#define __STO_STREAM_H__

#include "arda2/properties/proFirst.h"


#include "arda2/core/corStdString.h"
#include "arda2/properties/proStream.h"
#include "arda2/storage/stoFileOSFile.h"

/**
 * output stream for stoFile interface
**/
class stoStreamOutputFile : public proStreamOutput
{
public:
    stoStreamOutputFile();
    virtual ~stoStreamOutputFile();

    void Init(stoFile *file);

    virtual errResult Write( IN const char *data, IN uint size);

protected:
    stoFile *m_file;
};


/**
 * input stream for stoFile interface
**/
class stoStreamInputFile : public proStreamInput
{
public:
    stoStreamInputFile();
    virtual ~stoStreamInputFile();

    void Init(stoFile *file);

    virtual errResult Read( IN char *data, IN uint maxSize, OUT uint &bytesRead);

protected:
    stoFile *m_file;
};



#endif  // __STO_STREAM_H__

