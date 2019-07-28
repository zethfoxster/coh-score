#include "LogTime.h"
#include <iostream>

void LogTime::makeTime(log_time &ret, 
					   unsigned int year, 
					   unsigned int month, 
					   unsigned int day,
					   unsigned int hour, 
					   unsigned int minute, 
					   unsigned int second)
{
	ret = year%100;
	ret = ret *13 + month;
	ret = ret *35 + day;
	ret = ret *25 + hour;
	ret = ret *61 + minute;
	ret = ret *61 + second;
}

void LogTime::getTime(log_time time,
					  unsigned int &year, 
					  unsigned int &month, 
					  unsigned int &day,
					  unsigned int &hour, 
					  unsigned int &minute, 
					  unsigned int &second)
{
	second = time % 61;
	time /=61;
	minute = time % 61;
	time /=61;
	hour = time % 25;
	time /=25;
	day = time % 35;
	time /=35;
	month = time % 13;
	time /=13;
	year = time;
}

time_t LogTime::makeTimeT(log_time time)
{
	struct tm t;
	unsigned int year, month, day, hour, minute, second;
	getTime(time, year, month, day, hour, minute, second);
	t.tm_year = (year<50)?year+100:year;
	t.tm_mon = month;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	return mktime(&t);
}

log_time LogTime::makeTimeLog(time_t time)
{
	struct tm *t;
	log_time lTime;
	t = localtime(&time);
	makeTime(lTime, t->tm_year, t->tm_mon, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
	return lTime;
}

void LogTime::debugPrintTime(log_time time)
{
	unsigned int year, month, day, hour, minute, second;
	getTime(time, year, month, day, hour, minute, second);
	printf("%d:: %d %d %d %d::%d::%d\n", time, month, day, year, hour, minute, second);
}

bool LogTime::isDateReasonable(log_time input)
{
	static log_time tooEarly;
	static log_time tooLate;
	bool isInitialized = 0;
	if(!isInitialized)
	{
		isInitialized = true;
		LogTime::makeTime(tooEarly, 2000, 0, 0, 0, 0, 0);
		LogTime::makeTime(tooLate, 2050, 0, 0, 0, 0, 0);
	}
	return input > tooEarly && input < tooLate;
}