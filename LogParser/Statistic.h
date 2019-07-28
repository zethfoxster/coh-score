#ifndef _STATISTIC_H
#define _STATISTIC_H

#include "StashTable.h"

typedef struct Statistic
{
	int count;
	int total;
	__int64 rms;
	int max;
	int min;
} Statistic;

void  StatisticInit(Statistic * s);
void  StatisticRecord(Statistic * s, int val);
float StatisticGetAverage(const Statistic * s);
float StatisticGetVariance(const Statistic * s);
float StatisticGetStandardDeviation(const Statistic * s);


void RecordStashStatistic(StashTable table, char * key, int value);

#endif  // _STATISTIC_H