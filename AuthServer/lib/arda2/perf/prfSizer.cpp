//******************************************************************************
/**
 *   Created: 2005/04/11 
 * Copyright: 2005, NCSoft. All Rights Reserved
 *    Author: Tom C Gambill
 *
 *  @par Last modified
 *      $DateTime: 2007/12/12 10:43:51 $
 *      $Author: pflesher $
 *      $Change: 700168 $
 *  @file
 *
 *   Purpose:  Keeps track of sizes and allocations of objects
 */
//******************************************************************************

#include "arda2/core/corFirst.h"
#include "arda2/perf/prfSizer.h"
#include "arda2/core/corStlAlgorithm.h"
#include "arda2/core/corStlVector.h"

using namespace std;

#ifdef PERFSIZER_ENABLE

//* Define the global static monitor object
static prfSizer s_PerfSizer;


//*****************************************************************
//*  prfSizer()
//*
//*
//*****************************************************************
prfSizer::prfSizer()
{
    m_classNames.AllocateHeap(1024*4);
}


//*****************************************************************
//*  ~prfSizer()
//*
//*
//*****************************************************************
prfSizer::~prfSizer() 
{
    Dump();
}


inline bool DecreasingTotal( const prfSizer::ObjectData* pFirst, 
                            const prfSizer::ObjectData* pSecond)
{
    return (pSecond->iTotalAllocations < pFirst->iTotalAllocations);
}

//*****************************************************************
//*  Dump contents to std out
//*
//*
//*****************************************************************
void prfSizer::Dump() 
{
    corOutputDebugString("Dumping prfSizer Tracked Object Allocations:\n");

    // Sort by decreasing total time
    vector<ObjectData*> sortedObjectData;
    NameToObjectDataMap::iterator it = m_objectMap.begin();
    NameToObjectDataMap::iterator end = m_objectMap.end();
    for (;it!=end;++it)
    {
        sortedObjectData.push_back(&(it->second));
    }
    sort(sortedObjectData.begin(), sortedObjectData.end(), DecreasingTotal);

    vector<ObjectData*>::iterator itSorted = sortedObjectData.begin();
    vector<ObjectData*>::iterator endSorted = sortedObjectData.end();
    for (;itSorted!=endSorted;++itSorted)
    {
        ObjectData* pData = *itSorted;
        corOutputDebugString("  Total #: %d \tMax #: %d \tSize Each: %d \tSize Total: %d \tSize Max: %d \tRemaining: %d Object: %s \t\n",
            pData->iTotalAllocations,
            pData->iMaxActiveAllocations,
            pData->iObjectBytes,
            pData->iObjectBytes*pData->iTotalAllocations,
            pData->iObjectBytes*pData->iMaxActiveAllocations,
            pData->iActiveAllocations,
            pData->szObjectName);
    }
}


//******************************************************************************
/**
 * Take a Function Signature string and get the function and class name.
 *
 * @param szSignature - function signature string to get the class name from
 *
 * @return const char* to string allocated on the string heap
 **/
//******************************************************************************
const char* prfSizer::GetClassName(const char* szSignature)
{
    string strTemp = szSignature;
    size_t iScope = strTemp.rfind("::");
    if (iScope!=strTemp.npos)
    {
        return m_classNames.AddString(strTemp.substr(0, iScope).c_str());
    }

    return m_classNames.AddString(szSignature);
}


//******************************************************************************
/**
 * Add an object
 *
 *  @return void
 */
//******************************************************************************
void prfSizer::AddObject(const char* szSignature, uint iBytes)
{
    errAssert(szSignature);
    errAssert(iBytes);
    const char* szClassName = GetClassName(szSignature);
    NameToObjectDataMap::iterator it = m_objectMap.find(szClassName);
    if (it==m_objectMap.end())
    {
        // First one of these
        ObjectData data;
        data.szObjectName = szClassName;
        data.iActiveAllocations = 1;
        data.iTotalAllocations = 1;
        data.iMaxActiveAllocations = 1;
        data.iObjectBytes = iBytes;
        m_objectMap.insert(make_pair(szClassName, data));
    }
    else
    {
        // Add one
        ObjectData& data = it->second;
        ++data.iActiveAllocations;
        ++data.iTotalAllocations;

        if (data.iMaxActiveAllocations<data.iActiveAllocations)
            data.iMaxActiveAllocations = data.iActiveAllocations;
    }
}


//******************************************************************************
/**
 * Remove an object
 *
 *  @return void
 */
//******************************************************************************
void prfSizer::RemoveObject(const char* szSignature)
{
    errAssert(szSignature);
    const char* szClassName = GetClassName(szSignature);
    NameToObjectDataMap::iterator it = m_objectMap.find(szClassName);
    if (it==m_objectMap.end())
    {
        // This object was never added
        errAssert(0);
    }
    else
    {
        // Remove one
        ObjectData& data = it->second;
        --data.iActiveAllocations;

//        errAssert(data.iActiveAllocations>=0); // don't go negative
    }
}

#endif // PERFSIZER_ENABLE

