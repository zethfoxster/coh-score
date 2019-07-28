/*****************************************************************************
	created:	2001/10/05
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_timIntervalTimer
#define   INCLUDED_timIntervalTimer
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/timer/timTimer.h"

class timIntervalTimer
{
public:
	timIntervalTimer( uint frequency, uint interval = 0 ) : 
		m_timer(frequency),
		m_interval(interval)
	{
		Reset();
	};

	void SetInterval( uint interval )
	{
		m_interval = interval;
	}

	bool IsTriggered() const
	{
		return m_timer.GetElapsed() >= m_triggerTime;
	}

	void Reset()
	{
		m_triggerTime = m_interval;
		m_timer.Reset();
	}

	void ForceTrigger()
	{
		m_triggerTime = 0;
	}

private:
	timTimer		m_timer;
	uint			m_interval;
	uint			m_triggerTime;
};



#endif // INCLUDED_timIntervalTimer

