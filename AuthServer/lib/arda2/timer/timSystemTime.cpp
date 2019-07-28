/**
 *   Created: 2006/06/01
 *   Copyright: 2006, NCSoft. All Rights Reserved
 *   Author: Mike Howard
 *
 *  @par Last modified
 *      $DateTime: 2006/06/01 12:00:00 $
 *      $Author: mhoward $
 *      $Change: 1 $
 *  @file
 *
 */

#include "timSystemTime.h"
#if CORE_SYSTEM_LINUX
#include <time.h>
#include <sys/time.h>
#define		TIMEZONE	timezone
#else
#include <time.h>
#define		TIMEZONE	_timezone
#endif

SystemTime::SystemTime()
{
	GetTimeUTC();
}

SystemTime::SystemTime(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute, uint8 second, uint16 millisecond)
{
    m_year = year;
	m_month = month;
	m_day = day;
	m_hour = hour;
	m_minute = minute;
	m_second = second;
	m_millisecond = millisecond;
}

SystemTime::SystemTime(unsigned int currentTimeSeconds, unsigned int currentTimeMicroseconds)
{
	BuildTime(currentTimeSeconds, currentTimeMicroseconds/1000);
}

SystemTime::~SystemTime()
{
}

void SystemTime::SecondsAndMicroseconds(unsigned int &currentTimeSeconds, unsigned int &currentTimeMicroseconds) const
{
    struct tm tmIn;
    memset(&tmIn, 0, sizeof(tm));
    tmIn.tm_year = m_year - 1900;
    tmIn.tm_mon = m_month - 1;
    tmIn.tm_mday = m_day;
    tmIn.tm_hour = m_hour;
    tmIn.tm_min = m_minute;
    tmIn.tm_sec = m_second;

    currentTimeSeconds = static_cast<unsigned int>(mktime(&tmIn)) - TIMEZONE;
    currentTimeMicroseconds = m_millisecond * 1000;
}

void SystemTime::GetTimeUTC()
{
	uint32 sec;
	uint32 msec;
#if CORE_SYSTEM_LINUX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sec = tv.tv_sec;
	msec = tv.tv_usec / 1000;
#else
//#if CORE_SYSTEM_WIN32
	time_t sectime;
	SYSTEMTIME systime;
	time(&sectime);
	GetSystemTime(&systime);
	sec = static_cast<uint32>(sectime);
	msec = systime.wMilliseconds;
#endif 

	BuildTime(sec, msec);
}

void SystemTime::BuildTime(unsigned int sec, unsigned int milliseconds)
{
	struct tm tmOut;
	memset(&tmOut, 0, sizeof(tm));
	time_t tmp = static_cast<time_t>(sec);

#if CORE_SYSTEM_LINUX
	struct tm tmval;
	gmtime_r(&tmp, &tmval);
	tmOut = tmval;
#else
//#if CORE_SYSTEM_WIN32
	struct tm* ptmval = NULL;
	ptmval = gmtime(&tmp);
    if (ptmval != 0)
    {
	    tmOut = *ptmval;
    }
#endif

	m_year = static_cast<uint16>(tmOut.tm_year + 1900);
	m_month = static_cast<uint8>(tmOut.tm_mon + 1);
	m_day = static_cast<uint8>(tmOut.tm_mday);
	m_hour = static_cast<uint8>(tmOut.tm_hour);
	m_minute = static_cast<uint8>(tmOut.tm_min);
	m_second = static_cast<uint8>(tmOut.tm_sec);
	m_millisecond = static_cast<uint16>(milliseconds);
}

//void SystemTime::GetTimeLocal()
//{
//	uint32 sec;
//	uint32 msec;
//#if CORE_SYSTEM_LINUX
//	struct timeval tv;
//	gettimeofday(&tv, NULL);
//	sec = tv.tv_sec;
//	msec = tv.tv_usec / 1000;
//#else
////#if CORE_SYSTEM_WIN32
//	time_t sectime;
//	SYSTEMTIME systime;
//	time(&sectime);
//	GetLocalTime(&systime);
//	sec = static_cast<uint32>(sectime);
//	msec = systime.wMilliseconds;
//#endif 
//
//	BuildTime(sec, msec);
//}
