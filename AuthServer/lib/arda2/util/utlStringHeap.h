//******************************************************************************
/**
 *   Created: 2004/09/01
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

#ifndef __utlStringHeap
#define __utlStringHeap
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStlHashSet.h"
#include "arda2/memory/memExplicitHeap.h"

//******************************************************************************
/**
 * This object stores and maintains a collection of strings. It uses an explicit
 * heap to allocate the memory for the strings in a very efficient and compact
 * manner. This increases cache coherence and string lookup and comparison
 * performance.
 *
 * @ingroup Core Memory Allocation Classes
 * @brief String store to maintain a collection of strings with a similar 
 *        lifespan.
 */
//******************************************************************************
class utlStringHeap
{
public:
    utlStringHeap();
    ~utlStringHeap();

    /// Allocates memory, copies the string to it, then returns the new pointer
    const char* AddString(const char* szString);

    /// Must call to allocate the initial heap
    bool AllocateHeap(int iInitialHeapSize, bool bCheckUniqueness=true );
    void Release();

    bool  IsEmpty() { return m_stringHeap.IsEmpty(); }

    /// Dump strings to stdout
    void DumpContents();

protected:

private:
   memExplicitHeap m_stringHeap;

   struct corHashTraitsChar : public corHashTraitsBase<const char*>
   {
       static inline size_t Hash(const char* x)
       { return corStringHashFunc::Hash(x);}

       static inline bool Less(const char* lhs, const char* rhs)
       { return strcmp(lhs, rhs) < 0; }

       static inline bool Equal(const char* lhs, const char* rhs)
       { return strcmp(lhs, rhs) == 0; }
   };

   typedef HashSet<const char*, corHashTraitsChar> StringHashSet;
   StringHashSet   m_stringHash;

   bool            m_bCheckUniqueness;
};
#endif /* __utlStringHeap */



