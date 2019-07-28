/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef TIMER_H
#define TIMER_H

#include "ncstd.h"
NCHEADER_BEGIN

#define MS_PER_SEC	1000
#define SEC_PER_MIN	60
#define MIN_PER_HR	60
#define HR_PER_DAY	24

#define MS_PER_MIN	(MS_PER_SEC*SEC_PER_MIN)
#define MS_PER_HR	(MS_PER_SEC*SEC_PER_MIN*MIN_PER_HR)
#define MS_PER_DAY	(MS_PER_SEC*SEC_PER_MIN*MIN_PER_HR*HR_PER_DAY)

U32 time_ss2k();  // seconds since 2000
S64 time_mss2k(); // 1/1000ths of a seconds since 2000
U32 time_ss2k_delta(U32 old_time_ss2k);
int time_ss2k_checkandset(U32 *old_time_ss2k, U32 threshold);

char *curtime_str(char *dest, size_t n);


NCHEADER_END
#endif //TIMER_H
