#ifndef INCLUDED_SystemTime
#define INCLUDED_SystemTime
//******************************************************************************
/**
 *   Created: 2006/06/01
 *   Copyright: 2006, NCSoft. All Rights Reserved
 *   Author: Mike Howard
 * 
 *  @par Last modified
 *      $DateTime: 2006/06/01 16:16:58 $
 *      $Author: mhoward $
 *      $Change: 1 $
 *  @file
 *
 */
//******************************************************************************

#include "arda2/core/corFirst.h"

class SystemTime
{
public:
	SystemTime();
    SystemTime(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute, uint8 second, uint16 millisecond);
	SystemTime(unsigned int currentTimeSeconds, unsigned int currentTimeMicroseconds);
	virtual ~SystemTime();

	const uint16 Year() const {return m_year;}
	const uint8 Month() const {return m_month;}
	const uint8 Day() const {return m_day;}
	const uint8 Hour() const {return m_hour;}
	const uint8 Minute() const {return m_minute;}
	const uint8 Second() const {return m_second;}
	const uint16 Millisecond() const {return m_millisecond;}

    void SecondsAndMicroseconds(unsigned int &seconds, unsigned int &microseconds) const;

	void GetTimeUTC();
	//void GetTimeLocal();

private:
	uint16 m_year;
	uint8  m_month;
	uint8  m_day;
	uint8  m_hour;
	uint8  m_minute;
	uint8  m_second;
	uint16 m_millisecond;

	// disabled
	SystemTime(const SystemTime&);
	SystemTime operator= (const SystemTime);

	void BuildTime(unsigned int sec, unsigned int milliseconds);
};

#endif // INCLUDED_SystemTime

