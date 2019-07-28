/*****************************************************************************
	created:	2002/05/02
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	Error logger
*****************************************************************************/

#ifndef   INCLUDED_errErrorHandlerLogger
#define   INCLUDED_errErrorHandlerLogger
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corErrorHandler.h"

/**
 * file logger has been enhanced to handle appending to an existing file,
 * and daily rotation of that file. If a fileprefix is used in the constructor, 
 * todays date is appended to it for the file name. When the date changes, it 
 * rotates to a new file.
**/
class errErrorHandlerLogger : public corErrorHandler
{
public:

    enum ePathLogging
    {
        kFull = 0,
        kFilenames,
        kNone
    };

    enum eAppendLogging
    {
        kCreateFile = 0,
        kAppendToFile
    };

	errErrorHandlerLogger(bool useStdOut=false, ePathLogging hidePaths=kFull, const char *filePrefix = NULL, eAppendLogging append=kAppendToFile);
	~errErrorHandlerLogger();

	void Init(bool useStdOut=false, ePathLogging hidePaths=kFull, const char *filePrefix = NULL, eAppendLogging append=kAppendToFile);
	void SetUseStdOut(bool value) { m_useStdOut = value; }
	void SetHidePaths(ePathLogging value) { m_hidePaths = value; }

protected:  

    void RotateLogFile();

	errHandlerResult Report(const char* szFileName, int iLineNumber, 
		errSeverity nSeverity, const char *szErrorLevel, const char* szDescription);

	bool m_useStdOut;
	ePathLogging m_hidePaths;
    const char *m_filePrefix;
    FILE *m_fp;
    eAppendLogging m_append;
    int m_day; // used for log file rotation
    char m_logFileName[512];
};


#endif // INCLUDED_errErrorHandlerLogger

