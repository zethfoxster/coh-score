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

#ifndef   INCLUDED_prfSizer
#define   INCLUDED_prfSizer
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000   

//* define to enable perf monitor
#if CORE_DEBUG
//#define PERFSIZER_ENABLE
#endif

#if defined(PERFSIZER_ENABLE)

#include "arda2/util/utlSingleton.h"
#include "arda2/util/utlStringHeap.h"
#include "arda2/core/corStlMap.h"


//******************************************************************************
/**
 *  Keeps track of sizes and allocations of objects
 *
 * @ingroup Core Performance Monitoring/Optimization Helper Classes
 * @brief Allocation helper/tracker.
 */
//******************************************************************************
class prfSizer : public utlSingleton<prfSizer>
{
public:
    prfSizer();
    ~prfSizer();

    void AddObject(const char* szClassName, uint iBytes);
    void RemoveObject(const char* szClassName);

    void Dump();

    struct ObjectData
    {
        const char* szObjectName;
        int iTotalAllocations;     // Number of this type of object that have ever existed
        int iActiveAllocations;    // Number of this type of object that exist now
        int iMaxActiveAllocations; // Highest count
        int iObjectBytes;          // Size of one of these objects
    };
private:
    const char* GetClassName(const char* szSignature);

    typedef std::map< const char*, ObjectData > NameToObjectDataMap;
    NameToObjectDataMap m_objectMap;

    utlStringHeap       m_classNames;
};


//*****************************************************************************
//* Define the interface
//*
//* Macros are used here so the variables can be defined internally. This way
//* there is no additional client code needed to make this work. In 
//* non-intrumented builds, this code completely goes away.
//*****************************************************************************
#define PERFSIZER_ADDOBJECT()       if (prfSizer::GetSingletonPtr()) prfSizer::GetSingleton().AddObject(__FUNCTION__, sizeof(*this)); else corOutputDebugString("%s Initialized before prfSizer.\n", __FUNCTION__);
#define PERFSIZER_REMOVEOBJECT()    if (prfSizer::GetSingletonPtr()) prfSizer::GetSingleton().RemoveObject(__FUNCTION__); else corOutputDebugString("%s Initialized before prfSizer.\n", __FUNCTION__);

#else

#define PERFSIZER_ADDOBJECT()       {}
#define PERFSIZER_REMOVEOBJECT()    {}

#endif // PERFSIZER_ENABLE



#endif // INCLUDED_prfSizer

