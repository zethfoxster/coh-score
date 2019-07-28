/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "time.h"
#include "windows.h"
#include "timer.h"
#include "ncHash.h"


static S64 time_y2k()
{
	static S64 offset = 0;
    
	if (!offset)
	{
		SYSTEMTIME y2k = {0}; 
		y2k.wYear	= 2000;
		y2k.wMonth	= 1;
		y2k.wDay	= 1;
		SystemTimeToFileTime(&y2k,(FILETIME*)&offset);
	}
    
	return offset;
}

#define WINTICKSPERSEC 10000000
#define WINTICKSPERMSEC 10000

U32 time_ss2k()
{
	S64			x;
	U32			seconds;
 	GetSystemTimeAsFileTime((FILETIME *)&x);
	seconds = (U32)((x  - time_y2k()) / WINTICKSPERSEC);
    
	return seconds;
}

S64 time_mss2k()
{
	S64		x;
 	GetSystemTimeAsFileTime((FILETIME *)&x);
	return (x  - time_y2k()) / WINTICKSPERMSEC;
}

U32 time_ss2k_delta(U32 old_time_ss2k)
{
    return time_ss2k() - old_time_ss2k;
}

int time_ss2k_checkandset(U32 *old_time_ss2k, U32 threshold)
{
    assert(old_time_ss2k);
    if(time_ss2k_delta(*old_time_ss2k) >= threshold)
    {
        *old_time_ss2k = time_ss2k();
        return TRUE;
    }
    return FALSE;
}

char *curtime_str(char *dest, size_t n)
{
    time_t t = {0};    
    struct tm tm = {0}; 
    char *tmp;
    time(&t);
    gmtime_s(&tm,&t);
    asctime_s(dest,n,&tm);
    while((tmp = strrchr(dest,'\n'))!=NULL)
        *tmp = 0;
    return dest;
}
