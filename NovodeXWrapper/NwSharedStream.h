// -------------------------------------------------------------------------------------------------------------------
// NOVODEX SDK STREAM INTERFACE
// Written by Tom Lassanske, 03-01-05
// -------------------------------------------------------------------------------------------------------------------
#ifndef INCLUDED_NW_SHAREDSTREAM_H
#define INCLUDED_NW_SHAREDSTREAM_H

#include <stdio.h>

#include "Nxf.h"      // Definitions for all Nx types used here.
#include "NxStream.h" // Parent class.
// -------------------------------------------------------------------------------------------------------------------
class NwSharedStream : public NxStream
{
public:
    NwSharedStream(const char* streamName, bool load);
	NwSharedStream(const NxU8* pcStream, int iStreamSize, bool bCopy);
   virtual ~NwSharedStream();

    virtual bool isValid() const;

	void resetByteCounter();
	void checkTempBufferSize( NxU32 size);
	static void freeTempBuffer();

	NxU8* getBuffer();
	int	getBufferSize();
	void initWithCharBuffer(const NxU8* pcBuffer, int iBufferSize );


    virtual NxU8   readByte() const;
    virtual NxU16  readWord() const;
    virtual NxU32  readDword() const;
    virtual float  readFloat() const;
    virtual double readDouble() const;
    virtual void readBuffer(void* buffer, NxU32 size) const;

    virtual NxStream& storeByte(NxU8 a);
    virtual NxStream& storeWord(NxU16 a);
    virtual NxStream& storeDword(NxU32 a);
    virtual NxStream& storeFloat(NxReal a);
    virtual NxStream& storeDouble(NxF64 a);
    virtual NxStream& storeBuffer(const void* buffer, NxU32 size);


    static NxU8* pTempBuffer;
	static NxU32 uiBufferSize;
	mutable NxU8* pCursor;

};
// -------------------------------------------------------------------------------------------------------------------
#endif // INCLUDED_NW_SHAREDSTREAM_H