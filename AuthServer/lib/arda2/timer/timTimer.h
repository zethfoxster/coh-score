/*****************************************************************************
    created:    2001/08/07
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_timTimer
#define   INCLUDED_timTimer
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_SYSTEM_LINUX
#include <sys/time.h>
#endif

class timTimer
{
public:
    timTimer( uint frequency );

    static void InitializeSystemTimer();
    static uint64 GetSystemTimerValue();
    static float GetFloatSecondsNow();

    void Start();
    void Stop();
    uint GetElapsed() const;
    void Reset();

    float GetFloatSeconds();

private:
    uint64          m_remainder;
    uint64          m_start;
    uint64          m_stop;

    bool            m_running;
    uint64          m_divisor;          // tick divisor for this timer

    static bool     s_initialized;
    static uint64   s_systemFrequency;
};

//platform independent sleep
 void corSleep(int milliseconds);

// set the processor affinity to only use one cpu
uint32 timUseOneCPU();

#endif // INCLUDED_timTimer

