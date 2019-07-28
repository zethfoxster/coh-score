/**
    purpose:    
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Ryan Prescott
                Peter Freese
    modified:   $DateTime: 2005/11/17 10:07:52 $
                $Author: cthurow $
                $Change: 178508 $
                @file
**/

#ifndef   INCLUDED_utlSerialization
#define   INCLUDED_utlSerialization
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/storage/stoChunk.h"
#include "arda2/util/utlSize.h"
#include "arda2/util/utlPoint.h"
#include "arda2/util/utlRect.h"


template <typename T>
errResult Serialize( stoChunkFileWriter& writer, const T& t )
{
    return writer.Put(t);
}

template <typename T>
errResult Unserialize( stoChunkFileReader& reader, T& t )
{
    return reader.Get(t);
}


inline errResult Serialize( stoChunkFileWriter& writer, const utlSize& size )
{
    return writer.PutArray((uint32*)&size, 2);
}

inline errResult Unserialize( stoChunkFileReader& reader, utlSize& size )
{
    return reader.GetArray((uint32*)&size, 2);
}

inline errResult Serialize( stoChunkFileWriter& writer, const utlPoint& pt )
{
    return writer.PutArray((uint32*)&pt, 2);
}

inline errResult Unserialize( stoChunkFileReader& reader, utlPoint& pt )
{
    return reader.GetArray((uint32*)&pt, 2);
}

inline errResult Serialize( stoChunkFileWriter& writer, const utlRect& rect )
{
    return writer.PutArray( (int32*)&(rect), 4 );
}

inline errResult Unserialize( stoChunkFileReader& reader, utlRect& rect )
{
    return reader.GetArray( (int32*)&(rect), 4 );
}


#endif // INCLUDED_utlSerialization

