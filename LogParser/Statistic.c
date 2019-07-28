//
// Statistic.c		-	simple struct that stores max, min, total, count and rms values for a series of data
//						Call "RecordStatistic" with each value, and the structure will update itself accordingly
//

#include "Statistic.h"
#include "mathutil.h"
#include "StashTable.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>






void StatisticInit(Statistic * s)
{
	s->count = 0;
	s->total = 0;
	s->rms	 = 0;
	s->max	 = INT_MIN;
	s->min	 = INT_MAX;
}

float StatisticGetAverage(const Statistic * s)
{
	if(s->count)
		return ( ((float)s->total) / ((float)s->count));
	else
		return 0;
}

float StatisticGetVariance(const Statistic * s)
{
	float avg = StatisticGetAverage(s);

	float x;
	
	if(s->count > 1)
	{

		x = (s->rms) - (2 * avg * s->total) + (avg * avg * s->count);

		x /= (s->count - 1);		// "count - 1" gives UNBIASED std deviation 

	}
	else
	{
		return 0;
	}

	return x;
}

float StatisticGetStandardDeviation(const Statistic * s)
{
	float f = (float)sqrt(StatisticGetVariance(s));

	return f;
}

void StatisticRecord(Statistic * s, int val)
{
	s->count++;
	s->total += val;
	s->rms	 += ((__int64) val) * ((__int64) val);
	s->max	  = MAX(s->max, val);
	s->min	  = MIN(s->min, val);
}




void RecordStashStatistic(StashTable table, char * key, int value)
{
	Statistic * p;
	if(!stashFindPointer(table,key,&p))
	{
		p = malloc(sizeof(Statistic));
		StatisticInit(p);
		stashAddPointer(table, key, p, true);
	}

	StatisticRecord(p, value);
}


