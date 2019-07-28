/*****************************************************************************
created:    2002/05/02
copyright:  2002, NCSoft. All Rights Reserved
author(s):  Peter M. Freese

purpose:    Error logger
*****************************************************************************/

#include "arda2/core/corFirst.h"

#include "arda2/error/errErrorHandlerLogger.h"

#if CORE_SYSTEM_PS3
#include <time.h>
#else
#include <ctime>
#endif

errErrorHandlerLogger::errErrorHandlerLogger(
            bool useStdOut, 
            ePathLogging hidePaths, 
            const char *filePrefix,
            eAppendLogging append) : 
        m_useStdOut(useStdOut), 
        m_hidePaths(hidePaths),
        m_filePrefix(NULL),
        m_fp(NULL),
        m_append(append),
        m_day(-1)   // always change the first time through
{
    Init( useStdOut, hidePaths, filePrefix, append);
}

errErrorHandlerLogger::~errErrorHandlerLogger()
{
    if (m_filePrefix)
    {
        free((void*)m_filePrefix);
        m_filePrefix = NULL;
    }
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

void errErrorHandlerLogger::Init(
            bool useStdOut, 
            ePathLogging hidePaths, 
            const char *filePrefix,
            eAppendLogging append)
{
    m_useStdOut = useStdOut;
    m_hidePaths = hidePaths;
    m_append    = append;
    if (filePrefix)
    {
        m_filePrefix = strdup(filePrefix);
        RotateLogFile();
    }
}

errHandlerResult errErrorHandlerLogger::Report(const char* szFileName, int iLineNumber, 
                                            errSeverity /*nSeverity*/, const char *szSeverity, const char* szDescription)
{
	static char timeStamp[128] = {0};
	memset(timeStamp,0,128);

#if CORE_SYSTEM_LINUX
	static struct tm t;
	static time_t _t;
	time(&_t);
	localtime_r(&_t,&t);
	strftime(timeStamp,128,"%T",&t);
#elif CORE_SYSTEM_WINAPI
	_strtime(timeStamp);
#endif // CORE_SYSTEM_LINUX

#if CORE_SYSTEM_WINAPI
    // format a message

    corOutputDebugString("%s(%d): (%s) [%d] %s: %s\n", szFileName, iLineNumber, timeStamp, GetCurrentThreadId(), szSeverity, szDescription);
#endif

    if (!m_fp)
    {
        // dont log error message inside error log handler...
        return EH_Logged;
    }

    if (m_filePrefix)
    {
       RotateLogFile();
    }

    static char buf[1024];
    switch (m_hidePaths)
    {
    case kFull:
      {
        snprintf(buf, 1024, "%s(%d) : (%s) %s: %s\n", szFileName, iLineNumber, timeStamp, szSeverity, szDescription);
        break;
      }

    case kFilenames:
      {
        const char *p;
        p = (strrchr(szFileName, '\\'));
        if (p)
        {
            p++;
        } else {
            p = szFileName;
        }
        snprintf(buf, 1024, "%-25.25s(%6d) : (%s) %s: %s\n", p, iLineNumber, timeStamp, szSeverity, szDescription);
        break;
      }
    case kNone:
      {
          snprintf(buf, 1024, "(%s) %s: %s\n", timeStamp, szSeverity, szDescription);
      }
    }
    if (m_useStdOut)
        printf(buf);

    if (m_fp)
    {
        fprintf(m_fp, buf);
        fflush(m_fp);
    }

    return EH_Logged;
}


/**
 * RotateLogFile
 *
 *  called periodically to start a new log file with a new time-based name.
 *
**/
void errErrorHandlerLogger::RotateLogFile()
{
    int newYear;
    int newMonth;
    int newDay;
    FILE *newFile;

#if CORE_SYSTEM_WINAPI
   
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    newYear = sysTime.wYear;
    newMonth = sysTime.wMonth;
    newDay = sysTime.wDay;
#else

    struct tm *nowTM;
    time_t    now;
    time(&now);
    nowTM = localtime(&now);
    newYear = nowTM->tm_year + 1900;
    newMonth = nowTM->tm_mon + 1;
    newDay = nowTM->tm_mday;
#endif

    if (newDay != m_day)
    {
        // open the new file
        _snprintf(m_logFileName, 511, "%s_%4.4d_%2.2d_%2.2d.log", m_filePrefix, newYear, newMonth, newDay);
        if (m_append == kAppendToFile)
        {
            newFile = fopen(m_logFileName, "a");
        } else {
            newFile = fopen(m_logFileName, "w");
        }
        if (newFile)
        {
            // actually rotate to the new file
            if (m_fp)
            {
                fclose(m_fp);
            }
            m_day = newDay;
            m_fp = newFile;
        }
        else
        {
            perror("Unable to open new log file!");
        }
    }
}
