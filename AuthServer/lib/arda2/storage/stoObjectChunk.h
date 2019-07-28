#ifndef __PRO_OBJECT_CHUNK_H__
#define __PRO_OBJECT_CHUNK_H__

// constants and functions shared by the stoObjectReaderChunk
// and stoObjectWriterChunk classes

// used to label and interpret chunks
enum proObjectChunkTags
{
    kNULLTag =               0x0000,
    kObjectTag =             0x0001,
    kObjectReferenceTag =    0x0002,
    kSubObjectTag =          0x0004,
    kSubObjectReferenceTag = 0x0008,
};

// used to label and interpret property values in chunks
// (there is one for every intrinsic data type
enum proObjectChunkPropertyTags
{
    kChunkBase =             0x1000, // all chunk tags must include these bits and be larger
    kChunkBool =             0x1001,
    kChunkFloat =            0x1002,
    kChunkInt =              0x1004,
    kChunkObject =           0x1008,
    kChunkString =           0x1010,
    kChunkUint =             0x1020,
    kChunkWstring =          0x1040,
};

// the version number for objects with the PrOb TAG4 id
const uint stoChunkObjectVersion = 0;

#define STO_CHUNK_OBJECT_TAG 'PrOb'

#endif  // __PRO_OBJECT_CHUNK_H__
