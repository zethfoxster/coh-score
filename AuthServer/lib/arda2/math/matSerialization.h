/*****************************************************************************
    created:    2002/05/24
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Ryan M. Prescott
    
    purpose:    provide serialization services for math objects
*****************************************************************************/

#ifndef   INCLUDED_matserialization
#define   INCLUDED_matserialization
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matColor.h"
#include "arda2/math/matColorInt.h"
#include "arda2/math/matFarPosition.h"
#include "arda2/math/matFarTransform.h"
#include "arda2/math/matMatrix4x4.h"
#include "arda2/math/matPlane.h"
#include "arda2/math/matQuaternion.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matVector4.h"
#include "arda2/math/matTransform.h"
#include "arda2/storage/stoChunk.h"
#include "arda2/storage/stoSerialization.h"

inline errResult Unserialize( stoChunkFileReader &reader, matVector4 &vec ) 
{
    return reader.GetArray((float*)&vec, sizeof(matVector4)/ sizeof(float)); 
};

inline errResult Serialize( stoChunkFileWriter  &writer, const matVector4 &vec )
{ 
    return writer.PutArray((float*)&vec, sizeof(matVector4)/ sizeof(float)); 
};

inline errResult Unserialize( stoChunkFileReader &reader, matVector3 &vec )
{ 
    return reader.GetArray((float*)&vec, sizeof(matVector3)/ sizeof(float)); 
};

inline errResult Serialize( stoChunkFileWriter &writer, const matVector3 &vec )
{ 
    return writer.PutArray((float*)&vec, sizeof(matVector3)/ sizeof(float)); 
};

inline errResult Unserialize( stoChunkFileReader &reader, matPlane &pln )
{ 
    return reader.GetArray((float*)&pln, sizeof(matPlane)/ sizeof(float)); 
};

inline errResult Serialize( stoChunkFileWriter &writer, const matPlane &pln )
{ 
    return writer.PutArray((float*)&pln, sizeof(matPlane)/ sizeof(float)); 
};

inline errResult Unserialize( stoChunkFileReader &reader, matColor &clr )
{
    return reader.GetArray((float*)&clr, sizeof(matColor)/ sizeof(float)); 
};

inline errResult Serialize( stoChunkFileWriter &writer, const matColor &clr )
{ 
    return writer.PutArray((float*)&clr, sizeof(matColor)/ sizeof(float)); 
};

inline errResult Unserialize( stoChunkFileReader &reader, matColorInt &clr )
{
    return reader.GetArray((uint8*)&clr, sizeof(matColorInt)); 
};

inline errResult Serialize( stoChunkFileWriter  &writer, const matColorInt &clr )
{ 
    return writer.PutArray((uint8*)&clr, sizeof(matColorInt)); 
};

inline errResult Unserialize( stoChunkFileReader &reader, matFarSegment &seg )
{
    stoChunkFrameReader frame(reader);
    errResult er = ER_Success;

    if ( frame.GetTag() != 'FSEG' )
    {
        ERR_RETURN( ES_Error, "Invalid TAG unserializing matFarSegment!" );
    }

    switch (frame.GetVersion())
    {
        case 1:  // 2003-10-22 Piece Refactoring Initiative
        {
            er |= reader.Get(seg.x);
            er |= reader.Get(seg.y);
            er |= reader.Get(seg.z);
            break;
        }
        default:
        {
            ERR_RETURNV(ES_Error, ("Invalid chunk version (%i) unserializing matFarSegment", frame.GetVersion()));
        }
    }
    return er;
};

inline errResult Serialize( stoChunkFileWriter  &writer, const matFarSegment &seg )
{ 
    stoChunkFrameWriter frame(writer, 'FSEG', 1);
    writer.Put(seg.x);    
    writer.Put(seg.y);    
    writer.Put(seg.z);    
    return writer.GetResult();
};

inline errResult Unserialize( stoChunkFileReader &reader, matFarPosition &farpos )
{
    // legacy format with no chunk info
    if ( reader.PeekNextChunkTag() != 'FPOS' )
    {
#if CORE_COMPILER_MSVC
        // suppress non-standard extension warnings from the windows headers
#pragma warning(disable: 4201)
#endif

        union
        {
            struct
            {
                int16 x, z;
            };
            int32 xz;
        } oldSegment;
        matVector3 oldOffset;

#if CORE_COMPILER_MSVC
        // reenable non-standard extension warnings
#pragma warning(default: 4201)
#endif

        Unserialize(reader, oldSegment.xz);
        Unserialize(reader, farpos.m_offset);

        matFarSegment newSegment(oldSegment.x * 256, 0, oldSegment.z * 256);
        farpos.SetSegment(newSegment);
        farpos.Normalize();
        return reader.GetResult();
    }

    stoChunkFrameReader frame(reader);
    errResult er = ER_Success;

    if ( frame.GetTag() != 'FPOS' )
    {
        ERR_RETURN( ES_Error, "Invalid TAG unserializing matFarPosition!" );
    }

    switch (frame.GetVersion())
    {
        case 1:  // 2003-10-22 Piece Refactoring Initiative
        {
            er |= Unserialize(reader, farpos.m_segment);
            er |= Unserialize(reader, farpos.m_offset);
            break;
        }
        default:
        {
            ERR_RETURNV(ES_Error, ("Invalid chunk version (%i) unserializing matFarPosition", frame.GetVersion()));
        }
    }
    return reader.GetResult();
};

inline errResult Serialize( stoChunkFileWriter &writer, const matFarPosition &farpos )
{ 
    stoChunkFrameWriter frame(writer, 'FPOS', 1);
    Serialize( writer, farpos.m_segment );
    Serialize( writer, farpos.m_offset );
    return writer.GetResult();
};

inline errResult Unserialize( stoChunkFileReader &reader, matMatrix4x4 &mat )
{ 
    reader.GetArray(&mat._11, 4);
    reader.GetArray(&mat._21, 4);
    reader.GetArray(&mat._31, 4);
    reader.GetArray(&mat._41, 4);
    return reader.GetResult();
};

inline errResult Serialize( stoChunkFileWriter  &writer, const matMatrix4x4 &mat )
{ 
    writer.PutArray(&mat._11, 4);
    writer.PutArray(&mat._21, 4);
    writer.PutArray(&mat._31, 4);
    writer.PutArray(&mat._41, 4);
    return writer.GetResult();
}

inline errResult Unserialize( stoChunkFileReader &reader, matQuaternion &q ) 
{ 
    return reader.GetArray((float*)&q, sizeof(matQuaternion)/ sizeof(float)); 
};

inline errResult Serialize( stoChunkFileWriter &writer, const matQuaternion &q )
{ 
    return writer.PutArray((float*)&q, sizeof(matQuaternion)/ sizeof(float)); 
};

inline errResult UnserializeArray( stoChunkFileReader &reader, matQuaternion *pQuats, uint NumQuats )
{
    return reader.GetArray((float*)pQuats, NumQuats * sizeof(matQuaternion)/sizeof(float));
}

inline errResult SerializeArray( stoChunkFileWriter &writer, const matQuaternion* pQuats, uint NumQuats )
{
    return writer.PutArray((float*)pQuats, NumQuats * sizeof(matQuaternion)/sizeof(float));
}

inline errResult Unserialize( stoChunkFileReader &reader, matTransform &t )
{ 
    // legacy format with no chunk info
    // since GetScale returns the x component of scale, store
    // scalar scale into the x component
    if ( reader.PeekNextChunkTag() != 'MXFM' )
    {
        Unserialize(reader, t.m_quaternion);
        Unserialize(reader, t.m_position);
        Unserialize(reader, t.m_vScale.x);
        t.m_vScale.y = t.m_vScale.z = t.m_vScale.x;
        t.m_dirtyFlags.SetAll();
        ++t.m_iChangeStamp;
        return reader.GetResult();
    }

    stoChunkFrameReader frame(reader);
    errResult er = ER_Success;

    if ( frame.GetTag() != 'MXFM' )
    {
        ERR_RETURN( ES_Error, "Invalid TAG unserializing matTransform!" );
    }

    switch (frame.GetVersion())
    {
    case 1:     // 3/30/2004 first version to incorporate non-unif scaling info
        {       // changed from scalar scale to vector scale

            er |= Unserialize(reader, t.m_quaternion);
            er |= Unserialize(reader, t.m_position);
            er |= Unserialize(reader, t.m_vScale);
            break;
        }
    default:
        {
            ERR_RETURNV(ES_Error, ("Invalid chunk version (%i) unserializing matTransform", frame.GetVersion()));
        }
    }
    t.m_dirtyFlags.SetAll();
    ++t.m_iChangeStamp;
    return reader.GetResult();
};

inline errResult Serialize( stoChunkFileWriter &writer, const matTransform &t )
{
    // 4/8/2004 Addition of non-unif scaling info
    stoChunkFrameWriter frame(writer, 'MXFM', 1);
    Serialize(writer, t.m_quaternion);
    Serialize(writer, t.m_position);
    Serialize(writer, t.m_vScale);
    return writer.GetResult();
};

inline errResult Unserialize( stoChunkFileReader &reader, matFarTransform &t ) 
{
    // legacy format with no chunk info
    if ( reader.PeekNextChunkTag() != 'FXFM' )
    {
        matFarPosition pos;
        Unserialize(reader, pos);
        Unserialize(reader, t.m_localTransform);

        uint32 oldBasisSegment;
        Unserialize(reader, oldBasisSegment);

        t.SetPosition(pos);
        ++t.m_iChangeStamp;
        return reader.GetResult();
    }


    stoChunkFrameReader frame(reader);
    errResult er = ER_Success;

    if ( frame.GetTag() != 'FXFM' )
    {
        ERR_RETURN( ES_Error, "Invalid TAG unserializing matFarTransform!" );
    }

    switch (frame.GetVersion())
    {
        case 1:  // 2003-10-22 Piece Refactoring Initiative
        {
            er |= Unserialize(reader, t.m_position);
            er |= Unserialize(reader, t.m_localTransform);
            break;
        }
        default:
        {
            ERR_RETURNV(ES_Error, ("Invalid chunk version (%i) unserializing matFarTransform", frame.GetVersion()));
        }
    }
    ++t.m_iChangeStamp;
    return reader.GetResult();
};

inline errResult Serialize( stoChunkFileWriter  &writer, const matFarTransform &t )
{ 
    stoChunkFrameWriter frame(writer, 'FXFM', 1);
    Serialize(writer, t.m_position);
    Serialize(writer, t.m_localTransform);
    return writer.GetResult();
};

#endif // INCLUDED_matserialization

