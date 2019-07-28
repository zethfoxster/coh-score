/*****************************************************************************
	created:	2004/01/13
	copyright:	2004, NCSoft. All Rights Reserved
	author(s):	Ryan Prescott
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_utlGenericHash_h
#define   INCLUDED_utlGenericHash_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matConstants.h"

// namespace just used to prevent visibility at the global level
namespace utlGenericHashPrivate 
{

#define CHECKSTATE 8
#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

    // level is the hash function from a previous pass
    // if you're hashing multiple "elements" individually,
    // level should be the uint32 returned from the last element
    uint32  lookup(const uint8 *k, uint32 length, uint32 level);

    // this function operates on state, a 8x4byte array
    // yielding a 32 byte key
    // as the above, it should be used repeatedly for multi-element
    // structures, if they can't be hashed as one unit
    void    checksum(const uint8 *k, uint32 length, uint32 *state);

} // end namespace corGenericHashPrivate


/**
*  hash a generic data structure or array of structures
*  @param data pointer to the first element of the array
*  @param numElements the number of bytes in the array
*  @param prevKey used to pass in a key from a previous hash, 
*         if continuing the computation
*  @return hash value of data
*  @note potentially unsafe for structures containing dynamic 
*        values (pointers, vector<>, etc)
*/
inline uint32 utlGenericHash(   const void* data, 
                                uint32      sizeInBytes, 
                                uint32*     prevKey = NULL )
{    
    uint32 key = 0;
    if ( prevKey )
        key = *prevKey;

    uint32 newKey = utlGenericHashPrivate::lookup( (const uint8*)data, sizeInBytes, key );
    
    if ( prevKey )
        *prevKey = newKey;

    return newKey;
}


///**
//*  hash a generic data structure or array of structures
//*  @param data pointer to the first element of the array
//*  @param numElements the number of elements in the array
//*  @return hash value of data
//*  @note potentially unsafe for structures containing dynamic 
//*        values (pointers, vector<>, etc)
//*/
//template <typename T>
//uint32 utlGenericHash(T* data, int numElements = 1, uint32* prevKey = NULL)
//{
//    return utlGenericHash((uint8*)&data, sizeof(T)*numElements, prevKey);
//}

#endif // INCLUDED_utlGenericHash_h


