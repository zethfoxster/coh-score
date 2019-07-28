/**
 *  property system streams
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __STREAM_H__
#define __STREAM_H__


#include "arda2/core/corStdString.h"


/**
 * interface for output streams
**/
class proStreamOutput
{
public:
    proStreamOutput() {};
    virtual ~proStreamOutput() {};

    virtual errResult Write( IN const char *data, IN uint size) = 0;
};


/**
 * interface for input streams
**/
class proStreamInput
{
public:
    proStreamInput() {};
    virtual ~proStreamInput() {};

    virtual errResult Read( IN char *data, IN uint maxSize, OUT uint &bytesRead) = 0;
};



#endif  // __STREAM_H__

