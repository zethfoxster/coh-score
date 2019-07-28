#include "parse_util.h"
#include "SimpleParser.h"
#include "process_entry.h"
#include "mathutil.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>


const char * ParseEntry(const char * entry, int * time, char * mapName, char *mapIp, char * entityName, int * teamID, EntryProcessingFunc * func)
{
	char date[MAX_ENTRY_SIZE];
	int hour, min, sec;

	char actionStr[MAX_ENTRY_SIZE];
	const char * rest;
	const char * rest2;

	rest = ExtractTokens(entry, "%10000s %d:%d:%d %10000s:%I %10000n %d", date, &hour, &min, &sec, mapName, mapIp, entityName, teamID);

	if(!rest)
		return 0;

	*time = dateTime2Sec(date, hour, min, sec);

	rest2 = ExtractTokens(rest, "[%10000n]", actionStr);

	if(rest2)
	{
		*func = GetActionFunc(actionStr);
	}

	return rest2;
}

// avoids assert dialog when bad/crazy ascii values are tested with isdigit()
int safeIsDigit(int i)
{
	return ((unsigned)(i + 1) <= 256) &&  isdigit(i);
}

// avoids assert dialog when bad/crazy ascii values are tested with isalnum()
int safeIsAlnum(int i)
{
	return ((unsigned)(i + 1) <= 256) &&  isalnum(i);
}

int IsNameCharacter(int ch)
{
	return (   ch < 0  // allows handling of special name characters
			|| safeIsAlnum(ch)
			|| ch == ':'
			|| ch == '_'
			|| ch == '-'
			|| ch == '.'
			|| ch == '\''
			|| ch == '/'
			|| ch == '?');
}


int IsStringCharacter(int ch)
{
	return (   safeIsAlnum(ch)
			|| ch == '_'
			|| ch == '-'
			|| ch == '/'
			|| ch == '\\');
}

int IsIpCharacter(int ch)
{
	return (   safeIsAlnum(ch)
			|| ch == '.');
}



const char * ExtractTokens(const char * source, const char * fmt, ...)
{
	const char * pattern = fmt;
	va_list ap;

	pattern = removeLeadingWhiteSpaces(pattern);
	source = removeLeadingWhiteSpaces(source);

	va_start(ap, fmt);
	while(*pattern)
	{
		if(! *source)
		{
			//source was shorter than pattern
			va_end(ap);
			return 0;
		}
		//skip over whitespace
		if(isspace((unsigned char)*pattern))
		{
			pattern = removeLeadingWhiteSpaces(pattern);
			source = removeLeadingWhiteSpaces(source);
		}

		if(pattern[0] == '%')
		{
			int count = 0;

			if(safeIsDigit(pattern[1]))
			{
				// allow for # to precede 's' to specify max string size
				char buf[100];
				int i = 1;
				while(safeIsDigit(pattern[1]))
				{
					buf[i-1] = pattern[1];
					i++;
					pattern++;
				}
				buf[i-1] = '\0';
				count = atoi(buf);
			}

			if(pattern[1] == 's')
			{
				char * out = va_arg(ap, char*);
				source = ExtractStringToken(source, out, IsStringCharacter, count);
				pattern += 2;
			}
			else if(pattern[1] == 'I')
			{
                
				char * out = va_arg(ap, char*);
				source = ExtractStringToken(source, out, IsIpCharacter, count);
				pattern += 2;
			}
			else if(pattern[1] == 'n')
			{
				char * out = va_arg(ap, char*);
				source = ExtractStringToken(source, out, IsNameCharacter, count);
				removeTrailingWhiteSpaces(out);
				pattern += 2;
			}
			else if(pattern[1] == 'd')
			{
				int * out = va_arg(ap, int*);
				source = ExtractIntToken(source, out);
				pattern += 2;
			}
			else if(pattern[1] == 'f')
			{	
				float * out = va_arg(ap, float*);
				source = ExtractFloatToken(source, out);
				pattern += 2;
			}
			else
			{
				//ERROR!!!
				assert(!"Bad token specified!");
			}

			if(!source)
				return 0;
		}
		// skip over whitespace
		else if( *pattern != *source)
		{
			//printf("Source does not match pattern!!");
			va_end(ap);
			return 0;
		}
		else
		{
			++pattern;
			++source;
		}
	}

	va_end(ap);


	while(   (unsigned)(*source + 1) <= 256		// test for lower than '256' b/c some "Entity:Resume:Character" messges have trailing garbage onthe line (will fix for later logs)
		  && isspace((unsigned char)*source))
		++source;

	return source;
}

const char * copyQuotedString(char * out, const char * source, char end, int maxCount)
{
	int i = 0;

	assert(end == '"' || end == '\'' || end == ')');	// must begin with single or double quote
	
	source++;	// skip initial quote
	
	while(   source[i]
	      && source[i] != end)
	{
		i++;
	}

	if(maxCount > 0)
	{
		i = MIN(i, (maxCount - 1));
	}

	//assert(source[i]);  // otherwise, no terminating double-quote was found
	if(!source[i])
		return 0;	// bad format - no terminating double quote was found

	strncpy_s(out, 1000, source, i); 

	out[i] = '\0';

	source += (i + 1);  // skip over terminating double quote

	return source;
}

const char * ExtractStringToken(const char * source, char * out, int (*IsCharacterFunction) (int), int maxCount)
{
	char * temp = out; // debuggin only
	int i = 0;

	assert(source);
	assert(out);

	if(*source == '"' && !IsCharacterFunction('"'))
	{
		// copy everything up until end double-quote
		source = copyQuotedString(out, source, '"', maxCount);
	}
	else if(*source == '\'' && !IsCharacterFunction('\''))
	{
		// copy everything up until end single-quote
		// TODO
		source = copyQuotedString(out, source, '\'', maxCount);
	}
	else if(*source == '(' && !IsCharacterFunction(')'))
	{
		// copy everything up until end single-quote
		// TODO
		source = copyQuotedString(out, source, ')', maxCount);
	}
	else
	{
		int count = 0;
		while(     *source
		      &&   IsCharacterFunction(*source)
			  &&   ((maxCount == 0) || count < (maxCount -1)))
		{
			*(out++) = *(source++);
			count++;
		}

		*out = '\0';
	}

	return source;
}

const char * ExtractIntToken(const char * source, int * out)
{
	const char * p = source;
	char buf[100];
	char * str = &buf[0];

	if(   p[0] && p[1]  // RELACES: strlen(p) > 1
	   && (*p == '-'|| *p == '+')
	   && (safeIsDigit(p[1])))
	{
		*(str++) = *(p++);
	}

	while(safeIsDigit(*p))
	{
		*(str++) = *(p++);
	}

	if(p == source)
	{
		return 0;
	}
	
	*str = '\0';

	*out = atoi(buf);

	return p;
}



const char * ExtractFloatToken(const char * source, float * out)
{
	const char * p = source;
	char buf[100];
	char * str = &buf[0];
	int foundDecimal = 0;

	if(   p[0] && p[1]  // RELACES: strlen(p) > 1
		&& (*p == '-'|| *p == '+')
		&& (safeIsDigit(p[1])))
	{
		*(str++) = *(p++);
	}

	while(   ( !foundDecimal && (*p == '.'))
		  || safeIsDigit(*p))			// test for lower than '256' b/c some "Entity:Resume:Character" messges have trailing garbage onthe line (will fix for later logs)
	{
		if(*p == '.')
			foundDecimal = 1;

		*(str++) = *(p++);
	}

	if(p == source)
	{
		return 0;
	}

	*str = '\0';
   
	*out = (float) atof(buf);

	return p;
}




// set to 64K
#define BUF_SIZE 0x100
int getNextLine(FILE * pFile, char * line)
{
	static offset = 0;	
	static maxOffset = 0;
	static char buf [BUF_SIZE];

	int i = 0;
	int cr = 0;

	if(!pFile)
	{
		// reset everything
		offset		= 0;
		maxOffset	= 0;
		return 0;
	}


	while(i < (MAX_ENTRY_SIZE - 1))
	{
		if(offset >= maxOffset)
		{
			// fill up buffer
			if(-1 == (maxOffset = fread(buf, sizeof(char), BUF_SIZE, pFile)))
			{
				printf("Error reading file!\n");
				return 0;
			}

			if(maxOffset)
			{
				line[i] = buf[0];
				offset = 1;
			}
			else
			{
				line[i] = '\0';
				return 0;
			}
		}
		else
		{
			line[i] = buf[offset++];
		}

#ifdef CAREFUL		
		// Now needs a full \r\n to break
		if(cr && (line[i] == '\n'))
		{
			break;
		}
		cr = (line[i] == '\r');
#else
		if(line[i] == '\n')
		{
			break;
		}
#endif


		i++;
	}
	line[i] = '\0';
	return 1;
}
/*
// copied from UpdateServer code...
char *printUnit(char *buf,__int64 val)
{
	char		*units = "";

	if (val < 1000)
	{
		sprintf(buf,"%I64dB",val);
		return buf;
	}
	else if (val < 1000000)
	{
		val /= 100;
		units = "K";

	}
	else if (val < 1000000000)
	{
		val /= 100000;
		units = "M";
	}
	else if (val < 1000000000000)
	{
		val /= 100000000;
		units = "G";
	}
	else if (val < 1000000000000000)
	{
		val /= 100000000000;
		units = "T";
	}
	else if (val < 1000000000000000000)
	{
		val /= 100000000000000;
		units = "P";
	}
	if (val >= 1000)
		sprintf(buf,"%d%s",(int)(val/10),units);
	else
		sprintf(buf,"%d.%d%s",(int)(val/10),(int)(val%10),units);
	return buf;
}
*/
int dateTime2Sec(char * date, int hour, int min, int sec)
{
	int day, month, year;
	static int totalSec = 0;
	//	struct tm * pTime;
	struct tm curr;
	static struct tm prev;

	year	= digits2Number(&date[0], 2);
	month	= digits2Number(&date[2], 2);
	day		= digits2Number(&date[4], 2);

	curr.tm_year = (year + 100);	// equivalent to (year + 2000) - 1900	
	curr.tm_mon  = month - 1;	
	curr.tm_mday  = day;
	curr.tm_hour = hour;
	curr.tm_min = min;
	curr.tm_sec = sec;
	curr.tm_isdst = 0;

	// there seems to be a bug in this, so I disabled it...
	if(    0 && curr.tm_mday == prev.tm_mday
		&& curr.tm_mon  == prev.tm_mon
		&& curr.tm_year == prev.tm_year
		&& totalSec) // stored from last calculation
	{
		// increment off of last time value (to save expensive calls to mktime)
		totalSec += (  ( 3600 * (curr.tm_hour - prev.tm_hour))
			         + (   60 * (curr.tm_min  - prev.tm_min))
					 + (        (curr.tm_sec  - prev.tm_sec)));
	}
	else 
	{
		totalSec = (int) mktime(&curr);
	}

	prev = curr;

	return totalSec;
}


const char * sec2DateTime(time_t time)
{
	static char buf[1000];
	struct tm * pTM = localtime(&time);
	int len;

	strcpy_unsafe(buf, asctime(pTM));

	len = strlen(buf);
	buf[len-1] = '\0'; // get rid of newline

	return &(buf[0]);
}

const char * sec2Date(time_t time)
{
	static char buf[1000];
	struct tm * pTM = localtime(&time);
	const char * rest;
	char dummy[100], month[100], day[100];

	// quick, easy hack to get a month/day string
	strcpy_unsafe(buf, asctime(pTM));
	rest = ExtractTokens(buf, "%s %s %s", dummy, month, day);
	assert(rest);
	sprintf(buf, "%s %s", month, day);

	//len = strlen(buf);
	//buf[len-1] = '\0'; // get rid of newline

	return &(buf[0]);
}



int digits2Number(const char * numberString, int count)
{
	int i;
	char buf[MAX_DIGIT_LENGTH + 1];

	assert(count < MAX_DIGIT_LENGTH);

	for(i = 0; i < count; i++)
	{
		buf[i] = numberString[i];
	}

	buf[i] = '\0';

	return atoi(buf);
}


const char * int2str(int i)
{
	static char buf[100];
	itoa(i, buf, 10);
	return &buf[0];
}

const char * float2str(float f)
{
	static char buf[100];
	sprintf(buf, "%.2f", f);
	return &buf[0];
}

const char * sec2FormattedHours(int time)
{
	static char out[100];

	float hours = ((float) time) / 3600.0;

	sprintf(out, "%.2f", hours);
	return out;
}

char * sec2StopwatchTime(int time)
{
	static char out[100];

	sprintf(out, "%.2f", ((float) time) / 3600.0f);

	return out;
}

char * floatSec2StopwatchTime(float time)
{
	static char out[100];

	sprintf(out, "%.2f", time / 3600.0f);

	return out;
}

int stopwatchTime2Sec(const char * timeStr)
{
	int len = strlen(timeStr);
	int i;

	int hour, min, sec;

	// determine format
	int count = 0;
	for(i = 0; i < len; i++)
	{
		if(timeStr[i] == ':')
		{
			count++;
		}
	}

	if(count == 1)
	{
		// MM:SS format
		i = sscanf(timeStr, "%d:%d", &min, &sec);
		assert(i == 2);
		hour = 0;
	}
	else if(count == 2)
	{
		// HH:MM:SS format
		i = sscanf(timeStr, "%d:%d:%d", &hour, &min, &sec);
		assert(i == 3);
	}
	else
	{
		assert(!"Bad time string");
	}

	return (  (hour * 3600)
		    + (min  * 60  )
			+  sec        ); 

}

char * PercentString(int val, int total)
{
	static char buf[100];
	if(total)
		sprintf(buf, "%.2f%%", (100 * ((float)val / (float)total)));
	else
		strcpy_unsafe(buf, "0%");

	return (&buf[0]);
}


// strip the directory and extension from the file (copied from code in a Storyarc source code file)
void saUtilFilePrefix(char buffer[MAX_FILENAME_SIZE], char* source)
{
	char* pos;

	// clean the filename
	strcpy_unsafe(buffer, source);
	forwardSlashes(strupr(buffer));

	// strip the directory
	pos = strrchr(buffer, '/');
	if (pos)
	{
		pos++;
		memmove(buffer, pos, strlen(pos)+1);
	}

	// strip the extension
	pos = strrchr(buffer, '.');
	if (pos)
		*pos = 0;
}

void saUtilFileNoDir(char buffer[MAX_FILENAME_SIZE], char* source)
{
	char* pos;

	// clean the filename
	strcpy_unsafe(buffer, source);
	forwardSlashes(strupr(buffer));

	// strip the directory
	pos = strrchr(buffer, '/');
	if (pos)
	{
		pos++;
		memmove(buffer, pos, strlen(pos)+1);
	}
}


const char * GetOutDirName(time_t startTime, time_t endTime)
{
	static char buf[1000];
	struct tm * pTM;
	const char * rest;
	char dummy[100], startMonth[100], startDay[100], endMonth[100], endDay[100];

	// quick, easy hack to get a month/day string
	pTM = localtime(&startTime);
	strcpy_unsafe(buf, asctime(pTM));
	rest = ExtractTokens(buf, "%s %s %s", dummy, startMonth, startDay);
	assert(rest);

	pTM = localtime(&endTime);
	strcpy_unsafe(buf, asctime(pTM));
	rest = ExtractTokens(buf, "%s %s %s", dummy, endMonth, endDay);
	assert(rest);

	if(!strcmp(startMonth, endMonth))
	{
		// occurs in single month
		if(!strcmp(startDay, endDay))
		{
			// occurs in same day
			sprintf(buf, "%s %s", startMonth, startDay);
		}
		else
		{
			sprintf(buf, "%s %s - %s", startMonth, startDay, endDay);
		}
	}
	else
	{
		// occurs over 2+ month names
		sprintf(buf, "%s %s - %s %s", startMonth, startDay, endMonth, endDay);
	}	

	return &(buf[0]);
}


// This function accounts for +- (1,2 or 3) hour differences between consecutive times,
// as some machines may have clocks on a different time zone. It uses the last time as a reference,
// and expects to be initialized with a reference time. (Which is taken from the filename of the first logfile
// in a series).
// ab: this is f'in ridiculous
int CorrectTimeZone(int currTime, bool bReset)
{
	static int s_lastTime = 0;

	if(!bReset && s_lastTime)
	{
		int delta = currTime - s_lastTime;

		if( delta > ( 3.5 * 3600.0)
			|| delta < (-3.5 * 3600.0))
			printf("ERROR: more than +- 3.5 hours: $sLine\n");
		else if(delta > (2.5 * 3600.0))
			currTime -= (3 * 3600);
		else if(delta > (1.5 * 3600.0))
			currTime -= (2 * 3600);
		else if(delta > (0.75 * 3600.0))
			currTime -= (1 * 3600);
		else if(delta < (-2.5 * 3600.0))
			currTime += (3 * 3600);
		else if(delta < (-1.5 * 3600.0))
			currTime += (2 * 3600);
		else if(delta < (-0.75 * 3600.0))
			currTime += (1 * 3600);
	}
	
	s_lastTime = currTime;

	return currTime;
}



int getTimeFromFilename(const char * filename)
{
	int year, month, day, hour, min, sec;
	const char * rest;
	struct tm TimeStruct;
	char * p = strstr(filename, "db_");
	
	assert(p);

	rest = ExtractTokens(p, "db_%d-%d-%d-%d-%d-%d", &year, &month, &day, &hour, &min, &sec);
	assert(rest);

	TimeStruct.tm_year = (year - 1900);	
	TimeStruct.tm_mon  = month - 1;	
	TimeStruct.tm_mday  = day;
	TimeStruct.tm_hour = hour;
	TimeStruct.tm_min = min;
	TimeStruct.tm_sec = sec;
	TimeStruct.tm_isdst = 0;

	return mktime(&TimeStruct);
}
