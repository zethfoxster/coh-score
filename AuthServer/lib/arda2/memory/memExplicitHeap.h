//******************************************************************************
/**
 *   Created: 2004/08/31
 * Copyright: 2004, NCSoft. All Rights Reserved
 *    Author: Tom C Gambill
 *
 *  @par Last modified
 *      $DateTime: 2007/12/12 10:43:51 $
 *      $Author: pflesher $
 *      $Change: 700168 $
 *  @file
 *
 */
//******************************************************************************

#ifndef __memExplicitHeap
#define __memExplicitHeap
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlVector.h"


//******************************************************************************
/**
 * Implementation of an explicit memory heap. Allocates a large block of memory
 * that can be subsequently dished out in small pieces and then later freed as
 * one large block. It also has the ability to allocate additional blocks if the
 * original is filled, however it will never move a block once it is allocated.
 * The major advantages of this over calling ::new() for many small allocations
 * are that no tracking is done on the little blocks and allocation and freeing
 * memory in this heap is extremely fast, and there is not a 64-byte+ overhead
 * for each allocation as in the system heap.
 *
 * @ingroup Core Memory Allocation Classes
 * @brief Implementation of an Explicit Memory Heap.
 */
//******************************************************************************
class memExplicitHeap
{
public:
    memExplicitHeap();
    ~memExplicitHeap();

    /// Allocate a block of memory from the explicit heap
    byte* New(size_t iSize);

    bool  AllocateHeap(size_t iHeapSize, int iAlignment=8, bool bGrowable=false);
    void  Release();

    bool  IsEmpty() { return m_heaps.empty(); }

protected:

private:
    typedef std::vector<byte*> HeapVec;
    byte* AddHeapBlock();

    byte*     m_pNextAvailable;  ///< Pointer to the next available allocation
    intptr_t  m_iBytesRemaining; ///< number of free bytes remaining in the current block
    uintptr_t m_alignmentMask;

    bool      m_bGrowable;
    int       m_iAlignment;
    size_t    m_iHeapSize;

    HeapVec   m_heaps;         ///< Vector of blocks of allocated memory
};
#endif /* __memExplicitHeap */



