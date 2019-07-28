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
#include "arda2/util/utlStringHeap.h"
#include "arda2/memory/memMemory.h"

//#define DEBUG_STRING_HEAP

//******************************************************************************
/**
 *  Constructor
 *
 */
//******************************************************************************
utlStringHeap::utlStringHeap() :
    m_bCheckUniqueness(true)
{
}


//******************************************************************************
/**
 *  Destructor
 *
 */
//******************************************************************************
utlStringHeap::~utlStringHeap() 
{
    Release();
}


//******************************************************************************
/**
 * Allocate a heap
 *
 *  @param iInitialHeapSize - Size in the bytes of the initial allocation.
 *  @param bCheckUniqueness - Set to true to maintain a hash of the allocated
 *                            strings and return previous strings if found.
 *
 *  @return true if successful, false if not.
 */
//******************************************************************************
bool utlStringHeap::AllocateHeap(int iInitialHeapSize, bool bCheckUniqueness)
{
    // Already allocated?
    errAssertV(m_stringHeap.IsEmpty(), ("Heap already allocated!"));

    // Free any previous heap
    Release();

    m_bCheckUniqueness = bCheckUniqueness;
    return m_stringHeap.AllocateHeap(iInitialHeapSize, 4, true);
}


//******************************************************************************
/**
 * Allocate memory for a new string, copy the data to it, then return the new
 * pointer.
 *
 *  @param szString - string to store in the heap.
 *
 *  @return const char* to the new string
 */
//******************************************************************************
const char* utlStringHeap::AddString(const char* szString)
{
    char* szNewString;
    if (m_bCheckUniqueness)
    {
        // Search for this string in the set
        StringHashSet::iterator it = m_stringHash.find(szString);
        if ( it == m_stringHash.end() )
        {
            // Not found, need to allocate a new one
            size_t iSize = strlen(szString) + 1;
            if (iSize<=1)
                return 0; // No string

            szNewString = (char*)m_stringHeap.New(iSize);
            if (szNewString)
            {
                strncpy(szNewString, szString, iSize);
                m_stringHash.insert(szNewString);
            }
            return szNewString;
        }
        else
        {
            // Reuse the string
            return *it;
        }
    }
    else
    {
        // Not found, need to allocate a new one
        size_t iSize = strlen(szString) + 1;
        if (iSize<=1)
            return 0; // No string

        szNewString = (char*)m_stringHeap.New(iSize);
        if (szNewString)
            strncpy(szNewString, szString, iSize);

#ifdef DEBUG_STRING_HEAP
        m_stringHash.insert(szNewString);
#endif
        return szNewString;
    }
}


//******************************************************************************
/**
 * Release heap
 *
 *  @return void
 */
//******************************************************************************
void utlStringHeap::Release()
{
#ifdef DEBUG_STRING_HEAP
    DumpContents();
#endif

    m_stringHash.clear();
    m_stringHeap.Release();
}


//******************************************************************************
/**
 * Dump contents to std output
 *
 *  @return void
 */
//******************************************************************************
void utlStringHeap::DumpContents()
{
    if (!m_stringHash.empty())
    {
        ERR_REPORTV(ES_Info, ("utlStringHeap Contents: (%d char* strings)\n", (int)m_stringHash.size()));

        StringHashSet::iterator it = m_stringHash.begin();
        StringHashSet::iterator end = m_stringHash.end();
        for (;it!=end; ++it)
        {
            const char* sz = *it;
            corOutputDebugString("\t%s\n", sz);
        }
    }
}

