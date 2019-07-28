// Log.h: interface for the CLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOG_H__94A8AB42_336D_406E_813F_D28C162B57A8__INCLUDED_)
#define AFX_LOG_H__94A8AB42_336D_406E_813F_D28C162B57A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GlobalAuth.h"

enum LogType { 
	LOG_NORMAL, 
	LOG_VERBOSE,	// LOG_VERBOSE is enabled by "enableVerboseLogging" in config.txt
	LOG_DEBUG,		// Off in release builds by default, but can be turned on with "enableDebugLogging" in config.txt
	LOG_WARN, 
	LOG_ERROR
};

struct LogLine;

class CFileLog {         // No display. Write to file only.
public:
	CFileLog(const char *ext);
	~CFileLog();

	virtual bool SetDirectory(const char *logFolder);
	virtual void Enable(bool flag)	{ enabled = flag; }
			void SetMsgAllowed( LogType logType, bool bAllow );
			bool GetMsgAllowed( LogType logType );
	virtual void AddLog(LogType type, char* format, ...);
	virtual void ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz );
	virtual HANDLE OpenLogFile( const char* logFilePath );
	virtual bool StartNewLog( struct tm *tm )	{ return ( tm->tm_mday != prevDay || tm->tm_hour != prevHour || currFlag != prevFlag ); }

protected:
	void AddLogEx( LogType type, const char* format, va_list argList );
	bool SetDirectoryEx(const char *logFolder, struct tm *tm );
	DWORD LogTypeToBitMask( LogType logType )	{ return ( 1 << logType ); }

	CRITICAL_SECTION criticalSection;
	HANDLE logFile;
	bool enabled;
	int prevDay;
	int prevHour;
	int currFlag;
	int prevFlag;
	DWORD msgFilter;
	char fileExt[8];
	char logDirectory[_MAX_PATH];
};

class CLogdFilelog : public CFileLog 
{
public:
	CLogdFilelog( const char *ext);
	~CLogdFilelog();
	virtual bool SetDirectory(const char *logFolder);
	virtual void ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz );
	virtual void AddLog( LogType type, char *format, ... );
};

class CLog : public CFileLog	// writes to the app's log window
{
public:
//	CLog();
	CLog(int nLine, const char *ext);
	virtual ~CLog();
	void SetWnd( HWND hwnd );
	virtual void Enable( bool flag);
	virtual void ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz );
	virtual void AddLog(LogType type, const char* format, ...);

	void Resize(int width, int height);
	void Redraw();
private:
	HWND wnd;
	int nLine;
	int tail;
	LogLine* lines;
	RECT clientRect;
	SIZE fontSize;
	HFONT font;
	HBRUSH brush;
};


extern CLog log;
extern CFileLog filelog;
extern CFileLog actionlog;
extern CFileLog errlog;
extern CLogdFilelog logdfilelog;

// By default LOG_DEBUG and LOG_VERBOSE messages are turned off in release
// builds. You can override that in the config.txt file.
#define AS_LOG_DEBUG(...)	log.AddLog( LOG_DEBUG, __VA_ARGS__ )
#define AS_LOG_VERBOSE(...) log.AddLog( LOG_VERBOSE, __VA_ARGS__ )



#endif // !defined(AFX_LOG_H__94A8AB42_336D_406E_813F_D28C162B57A8__INCLUDED_)
