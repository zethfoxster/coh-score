// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"
#include "util.h"

struct LogLine {
	LogType type;
	char str[512];
	int len;
};

static COLORREF colors[] = { 
	RGB(  0,   0,   0),	// LOG_NORMAL
	RGB(  0,   0, 255),	// LOG_VERBOSE
	RGB(  0, 128, 128),	// LOG_DEBUG
	RGB(128,   0,   0),	// LOG_WARN
	RGB(255,   0,   0)	// LOG_ERROR
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLog log(128, "winlog");
CFileLog filelog("use");
CFileLog actionlog( "act" );
CFileLog errlog( "err" );
CLogdFilelog logdfilelog("log" );

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_arr_)	(sizeof(_arr_)/sizeof(_arr_[0]))
#endif


CFileLog::CFileLog(const char *ext)
  :	logFile(INVALID_HANDLE_VALUE), 
	enabled(false),
	prevDay(0),
	prevHour(0),
	currFlag(0),
	prevFlag(0),
	msgFilter(0)
{
	logDirectory[0]	= '\0';
	InitializeCriticalSectionAndSpinCount(&criticalSection, 4000);
	strcpy(fileExt, ext);
	
	// By default, these types are off.
	SetMsgAllowed( LOG_VERBOSE, false );
	SetMsgAllowed( LOG_DEBUG, false );
}

CFileLog::~CFileLog()
{
	DeleteCriticalSection(&criticalSection);
	if (logFile != INVALID_HANDLE_VALUE) {
		CloseHandle(logFile);
	}
}

void CFileLog::SetMsgAllowed( LogType logType, bool bAllow )
{
	// The types are zero-based scalar values.
	if ( bAllow )
	{
		// remove the bit to allow the message type
		msgFilter &= ~( LogTypeToBitMask(logType) );
	}
	else
	{
		// set the bit to prevent the message type
		msgFilter |= LogTypeToBitMask( logType );
	}
}

bool CFileLog::GetMsgAllowed( LogType logType )
{
	return  (( msgFilter & LogTypeToBitMask( logType )) == 0 );
}

void CFileLog::ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz )
{
	sprintf_s(pathBuff, pathBuffSz,
			"%s\\%04d-%02d-%02d.%02d.%s", 
			logDirectory, 
			tm->tm_year + 1900, 
			tm->tm_mon + 1, 
			tm->tm_mday, 
			tm->tm_hour, fileExt);
}

HANDLE CFileLog::OpenLogFile( const char* logFilePath )
{
	return CreateFile(logFilePath, GENERIC_WRITE, FILE_SHARE_READ,
								NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

bool CFileLog::SetDirectory(const char *logFolder)
{
	time_t t = time(0);
	struct tm *tm = localtime(&t);
	return SetDirectoryEx( logFolder, tm );
}

bool CFileLog::SetDirectoryEx(const char *logFolder, struct tm *tm )
{
	char logFileName[_MAX_PATH];
	strcpy( logDirectory, logFolder );
	ConstructLogPath( tm, logFileName, _MAX_PATH );
	if (( logFile = OpenLogFile( logFileName ) ) != INVALID_HANDLE_VALUE ) {
		SetFilePointer(logFile, 0, NULL, FILE_END);
	}
	prevDay = tm->tm_mday;
	prevHour = tm->tm_hour;
	
	return ( logFile != INVALID_HANDLE_VALUE );
}

void CFileLog::AddLogEx( LogType type, const char* format, va_list argList )
{
	if ( ! GetMsgAllowed(type) )
	{
		// We don't want these messages
		return;
	}

	time_t t = time(0);
	struct tm *tm = localtime(&t);

	EnterCriticalSection(&criticalSection);
	if ( StartNewLog( tm ) && logFile != INVALID_HANDLE_VALUE) {
		char logFileName[_MAX_PATH];
		ConstructLogPath( tm, logFileName, _MAX_PATH );
		HANDLE newLogFile = OpenLogFile( logFileName );
		HANDLE oldLog = (HANDLE)IntToPtr(InterlockedExchange((long *)&logFile, (long)PtrToInt(newLogFile)));
		if (oldLog)
			CloseHandle(oldLog);
		prevDay = tm->tm_mday;
		prevHour = tm->tm_hour;
	}
	LeaveCriticalSection(&criticalSection);

	char buffer[1024];
	int len = vsprintf_s(buffer, (ARRAY_SIZE(buffer)-2), format, argList);
	buffer[len++] = '\n';
	buffer[len] = '\0';
	DWORD writeBytes;
	if (len > 0) {
		if (logFile != INVALID_HANDLE_VALUE) {
			WriteFile(logFile, buffer, (DWORD)strlen(buffer), &writeBytes, NULL);
		}
	}

}

void CFileLog::AddLog(LogType type, char* format, ...)
{
	va_list argList;
	va_start(argList,format);
	AddLogEx( type, format, argList );
	va_end(argList);
}

CLogdFilelog::CLogdFilelog( const char *ext )
:CFileLog( ext )
{
}

CLogdFilelog::~CLogdFilelog()
{
}

void CLogdFilelog::AddLog( enum LogType logtype, char *format, ...)
{
_BEFORE
	time_t t = time(0);
	struct tm *tm = localtime(&t);
	currFlag = ( tm->tm_min < 30 ) ? 0 : 1;
	
	va_list argList;
	va_start(argList,format);
	AddLogEx( logtype, format, argList );
	va_end(argList);
	
	prevFlag = currFlag;
_AFTER_FIN
}

void CLogdFilelog::ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz )
{
	sprintf_s(pathBuff, pathBuffSz,
			"%s\\%04d-%02d-%02d-100%02d-201-authd-in%d.%s", 
			logDirectory, 
			tm->tm_year + 1900, 
			tm->tm_mon + 1, 
			tm->tm_mday, 
			tm->tm_hour,
			currFlag, 
			fileExt);
}

bool CLogdFilelog::SetDirectory( const char *logFolder )
{
	time_t t = time(0);
	struct tm *tm = localtime(&t);
	prevFlag = ( tm->tm_min < 30 ) ? 0 : 1;
	
	return SetDirectoryEx( logFolder, tm );
}

CLog::CLog(int n, const char *ext)
  : CFileLog(ext)
{
	nLine = n;
	lines = new LogLine [n];
	memset(lines, 0, sizeof(*lines) * n);
	tail = 0;
	font = 0;
}

CLog::~CLog()
{
	delete [] lines;
}

void CLog::SetWnd(HWND hwnd)
{
	wnd = hwnd;
}

void CLog::Redraw( void )
{
	if (!enabled) {
		return;
	}

	PAINTSTRUCT paint;
	HDC hdc = BeginPaint(wnd, &paint);

	if (font == 0) {
		font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
		SelectObject(hdc, font);
		GetClientRect(wnd, &clientRect);
		brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		GetTextExtentPoint32(hdc, "H", 1, &fontSize);
	} else {
		SelectObject(hdc, font);
	}

	int y = clientRect.bottom;
	int i = tail;
	RECT rect;
	do {
		y -= fontSize.cy;
		i = (i - 1) & (nLine - 1);
		if (lines[i].str) {
			SetTextColor(hdc, colors[lines[i].type]);
			TextOut(hdc, 0, y, lines[i].str, lines[i].len);
		}
		rect.left = lines[i].len * fontSize.cx;
		rect.right = clientRect.right;
		rect.top = y;
		rect.bottom = y + fontSize.cy;
		FillRect(hdc, &rect, brush);
	} while (y >= 0);

	EndPaint(wnd, &paint);
}

void CLog::Resize(int width, int height)
{
	clientRect.right = width;
	clientRect.bottom = height;
	if (enabled) {
		InvalidateRect(wnd, NULL, FALSE);
	}
}

void CLog::Enable( bool enableFlag )
{
	CFileLog::Enable( enableFlag );
	if (enableFlag) {
		InvalidateRect(wnd, NULL, FALSE);
	}
}

void CLog::ConstructLogPath( struct tm *tm, char* pathBuff, size_t pathBuffSz )
{
	sprintf_s(pathBuff, pathBuffSz, "%s\\%04d-%02d-%02d.%02d.%s", 
			logDirectory, 
			tm->tm_year + 1900, 
			tm->tm_mon + 1, 
			tm->tm_mday, 
			tm->tm_hour, 
			fileExt);
}

void CLog::AddLog( enum LogType logtype, const char *format, ... )
{
_BEFORE
	if ((!enabled) ||
		( ! GetMsgAllowed( logtype ))) {
		return;
	}

	time_t t = time(0);
	struct tm *tm = localtime(&t);
	
	{
		va_list argList;
		va_start(argList,format);
		AddLogEx( logtype, format, argList );
		va_end(argList);
	}

	char buffer[1024];
	sprintf_s(buffer, (sizeof(buffer)/sizeof(buffer[0])),
		"%02d.%02d.%02d %02d:%02d.%02d: ", tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	int timeStampLen = (int)strlen(buffer);
	va_list ap;
	va_start(ap, format);
	int len = vsprintf_s(buffer + timeStampLen, ((sizeof(buffer)/sizeof(buffer[0])) - timeStampLen), format, ap);
	va_end(ap);
	if (len > 0 ) {
		if ((len += timeStampLen) > sizeof(lines[tail].str)-2)
			len = sizeof(lines[tail].str)-2;

		EnterCriticalSection(&criticalSection);
		lines[tail].type = logtype;
		memcpy(lines[tail].str, buffer, len+1);
		lines[tail++].len = len;
		tail &= nLine - 1;
		LeaveCriticalSection(&criticalSection);
		if (enabled) 
			InvalidateRect(wnd, NULL, FALSE);
	}
_AFTER_FIN
}
