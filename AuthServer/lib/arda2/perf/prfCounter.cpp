//****************************************************************************
//*   Created:  2002/10/30
//* Copyright:  2002, NCSoft. All Rights Reserved
//* Author(s):  Tom C Gambill
//* 
//*   Purpose:  One node of a hierarchy of metric collecting objects used by
//*             gfxPerfMonitor
//* 
//****************************************************************************

#include "arda2/core/corFirst.h"
#include "arda2/perf/prfCounter.h"

using namespace std;

//*****************************************************************
//*  prfCounter()
//*
//*
//*****************************************************************
prfCounter::prfCounter(prfCounter* pParent, const char* szDescription) :
    m_bDisplayChildren(false),
    m_szDescription(szDescription),
    m_Timer(1000000), 
    m_pParent(pParent),
    m_uiElapsedTime(0),
    m_uiLastElapsedTime(0),
    m_uiAverageSumElapsedTime(0),
    m_uiTotalElapsedTime(0),
    m_uiMaxElapsedTime(0),
    m_iItemCount(0),
    m_iLastItemCount(0),
    m_iAverageSumItemCount(0),
    m_iTotalItemCount(0),
    m_iMaxItemCount(0),
    m_iFrameCount(0),
    m_iTotalFrameCount(0)
{
}


//*****************************************************************
//*  ~prfCounter()
//*
//*
//*****************************************************************
prfCounter::~prfCounter()
{
    //* Nuke all the children   
    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        prfCounter* pDoomedChild = *it;
        delete pDoomedChild;
    }
}


//*****************************************************************
//*  StartTimer()
//*
//*
//*****************************************************************
void prfCounter::StartTimer()
{
    m_Timer.Start();
}


//*****************************************************************
//*  StopTimer()
//*
//*
//*****************************************************************
void prfCounter::StopTimer()
{
    m_Timer.Stop();
    m_uiElapsedTime += m_Timer.GetElapsed();
}


//*****************************************************************
//*  BeginNewFrame()
//*
//*
//*****************************************************************
void prfCounter::BeginNewFrame()
{
    //* Move current values to last frames and average sums
    m_iLastItemCount = m_iItemCount;
    m_iTotalItemCount += m_iItemCount;
    m_iAverageSumItemCount += m_iItemCount;

    m_uiLastElapsedTime = m_uiElapsedTime;
    m_uiTotalElapsedTime += m_uiElapsedTime;
    m_uiAverageSumElapsedTime += m_uiElapsedTime;

    //* Does anything exceed the max values?
    if (m_iItemCount>m_iMaxItemCount)
        m_iMaxItemCount = m_iItemCount;
    if (m_uiElapsedTime>m_uiMaxElapsedTime)
        m_uiMaxElapsedTime = m_uiElapsedTime;

    //* Reset the counters for the next frame
    m_iItemCount = 0;
    m_uiElapsedTime = 0;

    //* New frame
    ++m_iFrameCount;
    ++m_iTotalFrameCount;

    //* call all the children
    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        (*it)->BeginNewFrame();
    }
}


//*****************************************************************
//*  Decrement()
//*
//*     Dec the current count for this node. Should always use a 
//*  static const string so the pointer is the same each time.
//*
//*****************************************************************
prfCounter* prfCounter::GetChild(const char* szDescription)
{
    //* Look for this child by name
    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        //* Compare the string pointers
        if ( (*it)->GetDescription() == szDescription )
            return *it;
    }

    //* Not found, create a new one and add it
    prfCounter* pNewChild = new prfCounter(this, szDescription);
    m_Children.push_back(pNewChild);

    return pNewChild;
}


//*****************************************************************
//*  ClearAverage()
//*
//*
//*****************************************************************
void prfCounter::ClearAverage()
{ 
    m_iAverageSumItemCount = 0; 
    m_uiAverageSumElapsedTime = 0; 
    m_iFrameCount = 0;

    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        (*it)->ClearAverage();
    }
}


//*****************************************************************
//*  ClearMaxElapsedTime()
//*
//*
//*****************************************************************
void prfCounter::ClearMaxElapsedTime()
{ 
    m_uiMaxElapsedTime = 0; 

    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        (*it)->ClearMaxElapsedTime();
    }
}


//*****************************************************************
//*  ClearMaxItemCount()
//*
//*
//*****************************************************************
void prfCounter::ClearMaxItemCount()
{ 
    m_iMaxItemCount = 0; 

    CounterVec::iterator it = m_Children.begin();
    CounterVec::iterator end = m_Children.end();
    for (; it!=end; ++it)
    {
        (*it)->ClearMaxItemCount();
    }
}

