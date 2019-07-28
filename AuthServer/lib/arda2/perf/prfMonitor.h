//****************************************************************************
//*   Created:  2002/10/30
//* Copyright:  2002, NCSoft. All Rights Reserved
//* Author(s):  Tom C Gambill
//* 
//*   Purpose:  Keeps track of various performance metrics
//* 
//****************************************************************************

#ifndef   INCLUDED_prfMonitor
#define   INCLUDED_prfMonitor
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000   

//* define to enable perf monitor
#define PERFMON_ENABLE

#if defined(PERFMON_ENABLE)

#include "arda2/util/utlSingleton.h"
#include "arda2/perf/prfCounter.h"


class prfMonitor : public utlSingleton<prfMonitor>
{
public:
    prfMonitor();
    ~prfMonitor();

    //* Create and/or open a sub-category within the current scope
    prfCounter* OpenCounter(const char* szUniqueName);

    //* Close the current counter scope, reverts to parent scope
    void            CloseCounter();

    //* This delimits a frame for per-frame stats and timers
    void            BeginNewFrame();

    prfCounter* GetCurrentCounter() { return m_pCurrentCounter; }
    prfCounter* GetRootCounter()    { return m_pRootCounter; }

private:
    prfCounter* m_pRootCounter;
    prfCounter* m_pCurrentCounter;
};


//* Uses scope to determine when to begin and end profiling at a certain scope.
struct prfScope
{
    prfScope(const char* szUniqueName)  
    { 
        pPerfMon = prfMonitor::GetSingletonPtr(); 
        pPerfMon->OpenCounter(szUniqueName); 
    }
    ~prfScope()                         
    {
        pPerfMon->CloseCounter(); 
    }

    prfMonitor* pPerfMon;
};

//* Uses scope to determine when to begin and end timing and report at a 
//* certain scope.
struct corTimeScope
{
    corTimeScope(const char* szFile, int iLine, const char* szFunc, const char* szDbgText) :
        m_Counter(0,"null")
    { 
        sprintf( m_buffer, "%s(%d): %s() - %s", szFile, iLine, szFunc, szDbgText);
        corOutputDebugString("%s *** Begin Timing ***\n", m_buffer);
        m_Counter.StartTimer();
    }
    ~corTimeScope()                         
    {
        m_Counter.StopTimer(); 
        corOutputDebugString("%s *** Time in Function = %1.3fms ***\n", m_buffer, (float)m_Counter.GetElapsedTime()*0.001f);
    }

    char m_buffer[256];
    prfCounter m_Counter;
};


//*****************************************************************************
//* Define the interface
//*
//* Macros are used here so the variables can be defined internally. This way
//* there is no additional client code needed to make this work. In 
//* non-intrumented builds, this code completely goes away.
//*****************************************************************************
#define PERFMON_PROFILE_BEGINNEWFRAME()         prfMonitor::GetSingleton().BeginNewFrame();

#define PERFMON_PROFILE(szUniqueIndentifier)    prfScope scopeCounter(szUniqueIndentifier);

#define PERFMON_ADD_ITEMS(iNumber)              scopeCounter.pPerfMon->GetCurrentCounter()->AddItems(iNumber); 
#define PERFMON_SUBTRACT_ITEMS(iNumber)         scopeCounter.pPerfMon->GetCurrentCounter()->SubtractItems(iNumber); 
#define PERFMON_SHOW_CHILDREN(boolShow)         scopeCounter.pPerfMon->GetCurrentCounter()->SetDisplayChildren(boolShow);

#define PERFMON_TIME_FUNC(szDbgText)            corTimeScope scopeTimer(__FILE__, __LINE__, __FUNCTION__, szDbgText);

#else


//* Everything goes away in normal builds
#define PERFMON_PROFILE_BEGINNEWFRAME()         {}

#define PERFMON_PROFILE(szUniqueIndentifier)    {}

#define PERFMON_ADD_ITEMS(iNumber)              {}
#define PERFMON_SUBTRACT_ITEMS(iNumber)         {}

#define PERFMON_SHOW_CHILDREN(boolShow)         {}
#define PERFMON_TIME_FUNC(szDbgText)            {}

#endif // PERFMON_ENABLE



#endif // INCLUDED_prfMonitor

