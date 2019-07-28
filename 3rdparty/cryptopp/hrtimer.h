#ifndef CRYPTOPP_HRTIMER_H
#define CRYPTOPP_HRTIMER_H

#include "config.h"

NAMESPACE_BEGIN(CryptoPP)

#if !defined(NO_OS_DEPENDENCE) && defined(WORD64_AVAILABLE) && (defined(_WIN32) || defined(__unix__) || defined(macintosh))
#define HIGHRES_TIMER_AVAILABLE
#endif

#ifdef HIGHRES_TIMER_AVAILABLE

//! high resolution timer
class Timer
{
public:
	enum Unit {SECONDS, MILLISECONDS, MICROSECONDS};
	Timer(Unit unit)	: m_started(false), m_timerUnit(unit) {}

	static word64 GetCurrentTimerValue();	// GetCurrentTime is a macro in MSVC 6.0
	static unsigned long ConvertTo(word64 t, Unit unit);

	// this is not the resolution, just a conversion factor into milliseconds
	static inline unsigned int TicksPerMillisecond()
	{
#if defined(_WIN32)
		return 10000;
#elif defined(__unix__) || defined(macintosh)
		return 1000;
#endif
	}

	void StartTimer();
	unsigned long ElapsedTime();

private:
	Unit m_timerUnit;	// HPUX workaround: m_unit is a system macro on HPUX
	bool m_started;
	word64 m_start;
};

#endif

NAMESPACE_END

#endif
