// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SDK STREAM INTERFACE
// Written by Tom Lassanske, 03-01-05
// -------------------------------------------------------------------------------------------------------------------
#ifndef INCLUDED_NW_STREAM_H
#define INCLUDED_NW_STREAM_H

#include <stdio.h>

#include "Nxf.h"      // Definitions for all Nx types used here.
#include "NxStream.h" // Parent class.
// -------------------------------------------------------------------------------------------------------------------
class NwStream : public NxStream
{
public:
    NwStream(const char* filename, bool load);
    virtual ~NwStream();

    virtual bool isValid() const;

    virtual NxU8   readByte() const;
    virtual NxU16  readWord() const;
    virtual NxU32  readDword() const;
    virtual float  readFloat() const;
    virtual double readDouble() const;
    virtual void readBuffer(void* buffer, NxU32 size) const;

    virtual NxStream& storeByte(NxU8 b);
    virtual NxStream& storeWord(NxU16 w);
    virtual NxStream& storeDword(NxU32 d);
    virtual NxStream& storeFloat(NxReal f);
    virtual NxStream& storeDouble(NxF64 f);
    virtual NxStream& storeBuffer(const void* buffer, NxU32 size);

    FILE* fp;
};
// -------------------------------------------------------------------------------------------------------------------
#endif // INCLUDED_NW_STREAM_H