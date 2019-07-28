#pragma once
#include <time.h>

typedef unsigned long long log_time;

class LogTime
{
private:
	LogTime();
public:
	static void makeTime(log_time &ret, 
		unsigned int year, 
		unsigned int month, 
		unsigned int day,
		unsigned int hour, 
		unsigned int minute, 
		unsigned int second);

	static void getTime(log_time time,
		unsigned int &year, 
		unsigned int &month, 
		unsigned int &day,
		unsigned int &hour, 
		unsigned int &minute, 
		unsigned int &second);

	static time_t makeTimeT(log_time time);

	static log_time makeTimeLog(time_t time);

	static void debugPrintTime(log_time time);

	static bool isDateReasonable(log_time input);
};
