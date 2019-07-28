//****************************************************************************
//*   Created:  2002/10/30
//* Copyright:  2002, NCSoft. All Rights Reserved
//* Author(s):  Tom C Gambill
//* 
//*   Purpose:  One node of a hierarchy of metric collecting objects used by
//*             gfxPerfMonitor
//* 
//****************************************************************************

#ifndef   INCLUDED_prfCounter
#define   INCLUDED_prfCounter
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000   

#include "arda2/core/corStlVector.h"
#include "arda2/timer/timTimer.h"

class prfCounter
{
public:
    prfCounter(prfCounter* pParent, const char* szDescription);
    ~prfCounter();

    //* Start Timing
    void StartTimer();

    //* Stop Timing
    void StopTimer();

    //* Add or remove a number to the current count for this node
    void AddItems(int iNumberToAdd)                 { m_iItemCount += iNumberToAdd; }
    void SubtractItems(int iNumberToSubtract)       { m_iItemCount -= iNumberToSubtract; }

    //* Processes the data for this frame, calls children too
    void BeginNewFrame();

    //* Navigation
    prfCounter* GetChild(const char* szDescription);
    prfCounter* GetChild(int iChild) const  { return m_Children[iChild]; }
    size_t          GetNumChildren() const      { return m_Children.size(); }
    prfCounter* GetParent() const           { return m_pParent; }
    const char*     GetDescription() const      { return m_szDescription; }
    const timTimer& GetTimer() const            { return m_Timer; }

    //* Timers
    uint            GetElapsedTime() const      { return m_uiElapsedTime; }         //* So far this frame
    uint            GetLastElapsedTime() const  { return m_uiLastElapsedTime; }     //* Total last frame
    uint            GetTotalElapsedTime() const { return m_uiTotalElapsedTime; }    //* Total since we started timing
    
    uint            GetMaxElapsedTime() const   { return m_uiMaxElapsedTime; }      //* Max time in a frame since the last ClearMaxElapsedTime()
    void            ClearMaxElapsedTime();                                          //* Clear the Max timer, recursive

    //* Item counters for things like VBs, memory, number of calls to a function, etc.
    int             GetItemCount() const        { return m_iItemCount; }            //* So far this frame
    int             GetLastItemCount() const    { return m_iLastItemCount; }        //* Total last frame
    int             GetTotalItemCount() const   { return m_iTotalItemCount; }       //* Total since we started counting
    
    int             GetMaxItemCount() const     { return m_iMaxItemCount; }         //* Max count in a frame since the last ClearMaxItemCount()
    void            ClearMaxItemCount();                                            //* Clear the Max counts, recursive

    //* Number of frames
    int             GetFrameCount() const       { return m_iFrameCount; }           //* Since the last ClearAverages()
    int             GetTotalFrameCount() const  { return m_iTotalFrameCount; }      //* Total since we started counting

    //* Averages
    uint            GetAverageElapsedTime() const       { return m_iFrameCount ? m_uiAverageSumElapsedTime/m_iFrameCount : 0; }
    int             GetAverageItemCount() const         { return m_iFrameCount ? m_iAverageSumItemCount/m_iFrameCount : 0; }
    uint            GetTotalAverageElapsedTime() const  { return m_iTotalFrameCount ? m_uiTotalElapsedTime/m_iTotalFrameCount : 0; }
    int             GetTotalAverageItemCount() const    { return m_iTotalFrameCount ? m_iTotalItemCount/m_iTotalFrameCount : 0; }
    void            ClearAverage(); //* Clear the Average counts, recursive

    // Display of results
    void            ToggleDisplayChildren() const      { m_bDisplayChildren = !m_bDisplayChildren; }
    bool            GetDisplayChildren() const         { return m_bDisplayChildren; }
    void            SetDisplayChildren(bool val)       { m_bDisplayChildren = val; }

protected:
    prfCounter();

private:
    typedef std::vector<prfCounter*> CounterVec;

    mutable bool    m_bDisplayChildren;
    const char*     m_szDescription;
    timTimer        m_Timer;
    prfCounter* m_pParent;
    CounterVec      m_Children;

    //* Timers
    uint            m_uiElapsedTime;            //* So far this frame
    uint            m_uiLastElapsedTime;        //* Total last frame
    uint            m_uiAverageSumElapsedTime;  //* Total since last clear
    uint            m_uiTotalElapsedTime;       //* Total since we started timing
    uint            m_uiMaxElapsedTime;         //* Max time in a frame since the last ClearMaxElapsedTime()

    //* Item counters for things like VBs, memory, number of calls to a function, etc.
    int             m_iItemCount;               //* So far this frame
    int             m_iLastItemCount;           //* Total last frame
    int             m_iAverageSumItemCount;     //* Total since last clear
    int             m_iTotalItemCount;          //* Total since we started counting
    int             m_iMaxItemCount;            //* Max count in a frame since the last ClearMaxItemCount()

    //* Number of frames
    int             m_iFrameCount;              //* Since the last ClearAverages()
    int             m_iTotalFrameCount;         //* Total since we started counting
};



#endif // INCLUDED_prfCounter

