//******************************************************************************
/**
 *   Created: 2004/08/31
 * Copyright: 2004, NCSoft. All Rights Reserved
 *    Author: Tom C Gambill
 *
 *  @par Last modified
 *      $DateTime: 2005/11/17 10:07:52 $
 *      $Author: cthurow $
 *      $Change: 178508 $
 *  @file
 *
 */
//******************************************************************************

#include "arda2/core/corFirst.h"
#include "arda2/core/corCast.h"
#include "arda2/memory/memExplicitHeap.h"
#include "arda2/memory/memMemory.h"


//******************************************************************************
/**
 *  Constructor
 *
 */
//******************************************************************************
memExplicitHeap::memExplicitHeap() :
    m_pNextAvailable(0),
    m_iBytesRemaining(0),
    m_alignmentMask(0),
    m_bGrowable(false),
    m_iAlignment(8),
    m_iHeapSize(0)
{
}


//******************************************************************************
/**
 *  Destructor
 *
 */
//******************************************************************************
memExplicitHeap::~memExplicitHeap() 
{
    Release();
}


//******************************************************************************
/**
 * Allocate a heap
 *
 *  @return true if successful, false if not.
 */
//******************************************************************************
bool memExplicitHeap::AllocateHeap(size_t iInitialSize, int iAlignment, bool bGrowable)
{
    // Already allocated?
    errAssertV(m_heaps.empty(), ("Heap already allocated!"));

    // Free any previous heap
    Release();

    m_bGrowable = bGrowable;
    m_iAlignment = iAlignment;
    m_alignmentMask = (uintptr_t)(iAlignment-1);
    m_iHeapSize = iInitialSize;

    return AddHeapBlock() ? true : false;
}


//******************************************************************************
/**
 * Allocate a memory block
 *
 *  @return void*
 */
//******************************************************************************
byte* memExplicitHeap::New(size_t iSize)
{
    // Align and save the new address
    byte* pAlignedAllocation = (byte*)((uintptr_t)(m_pNextAvailable+m_alignmentMask)&(~m_alignmentMask));

    // Subtract used bytes
    m_iBytesRemaining -= (int)(pAlignedAllocation - m_pNextAvailable) + iSize;

    if (m_iBytesRemaining<0)
    {
        if (m_bGrowable)
        {
            // Add a new heap block
            ERR_REPORTV(ES_Info, ("Explicit Heap Growing"));
            pAlignedAllocation = AddHeapBlock();
            if (!pAlignedAllocation)
                return pAlignedAllocation;
        
            m_iBytesRemaining -= iSize; 
        }
        else
        {
            errAssertV(0, ("Explicit Heap Empty!"));
            return 0;
        }
    }

    // Set the new next point
    m_pNextAvailable = pAlignedAllocation + iSize;

    return pAlignedAllocation;
}


//******************************************************************************
/**
 * Release heaps
 *
 *  @return void
 */
//******************************************************************************
void memExplicitHeap::Release()
{
    HeapVec::iterator it = m_heaps.begin();
    HeapVec::iterator end = m_heaps.end();
    for (;it!=end; ++it)
    {
        FreeAligned(*it);
    }
    m_heaps.clear();

    m_pNextAvailable = 0;
    m_iBytesRemaining = 0;
    m_bGrowable = false;
    m_iAlignment = 8;
    m_iHeapSize = 0;
}


//******************************************************************************
/**
* Allocate a new heap
*
*  @return void* 
*/
//******************************************************************************
byte* memExplicitHeap::AddHeapBlock()
{
    // Make a heap and init the params
    byte* pHeap = (byte*)MallocAligned(m_iHeapSize, m_iAlignment);
    if (pHeap)
    {
        m_pNextAvailable = pHeap;
        m_iBytesRemaining = m_iHeapSize;

        m_heaps.push_back(pHeap);
    }

    return pHeap;
}


