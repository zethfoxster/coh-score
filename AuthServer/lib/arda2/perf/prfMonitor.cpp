//****************************************************************************
//*   Created:  2002/10/30
//* Copyright:  2002, NCSoft. All Rights Reserved
//* Author(s):  Tom C Gambill
//* 
//*   Purpose:  Keeps track of various performance metrics
//* 
//****************************************************************************

#include "arda2/core/corFirst.h"
#include "arda2/perf/prfMonitor.h"

#ifdef PERFMON_ENABLE

//* Define the global static monitor object
static prfMonitor s_PerfMonitor;


//*****************************************************************
//*  prfMonitor()
//*
//*
//*****************************************************************
prfMonitor::prfMonitor() :
    m_pRootCounter(0),
    m_pCurrentCounter(0)
{
    char szFilename[_MAX_PATH];
#if CORE_PLATFORM_WIN32||CORE_PLATFORM_WIN64
    GetModuleFileName(0,szFilename,_MAX_PATH);
#else
    sprintf(szFilename, "Palantir Core");
#endif
    m_pRootCounter = m_pCurrentCounter = new prfCounter(0, "Palantir");
    m_pRootCounter->ToggleDisplayChildren();
}


//*****************************************************************
//*  ~prfMonitor()
//*
//*
//*****************************************************************
prfMonitor::~prfMonitor() 
{
    delete m_pRootCounter;
    m_pRootCounter = 0;
    m_pCurrentCounter = 0;
}


//*****************************************************************
//*  OpenCounter()
//*
//*     This delimits a frame for per-frame stats and timers
//*
//*****************************************************************
void prfMonitor::BeginNewFrame()
{
    //* Delimit the root counter that always exists
    m_pRootCounter->StopTimer();
    m_pRootCounter->StartTimer();

    //* Parse the tree delimit
    m_pRootCounter->BeginNewFrame();
}


//*****************************************************************
//*  OpenCounter()
//*
//*     Create and/or open a sub-category
//*
//*****************************************************************
prfCounter* prfMonitor::OpenCounter(const char* szUniqueName)
{
    m_pCurrentCounter = m_pCurrentCounter->GetChild(szUniqueName);
    m_pCurrentCounter->StartTimer();
    
    return m_pCurrentCounter;
}


//*****************************************************************
//*  CloseCounter()
//*
//*     Close the current counter scope, reverts to parent scope
//*
//*****************************************************************
void prfMonitor::CloseCounter() 
{
    //* Just in case
    m_pCurrentCounter->StopTimer();

    prfCounter* pParentCounter = m_pCurrentCounter->GetParent();
    if (pParentCounter)
        m_pCurrentCounter = pParentCounter;
}

#endif // PERFMON_ENABLE

