#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include "stdtypes.h"

C_DECLARATIONS_BEGIN

extern const char* build_version;

typedef void (*ErrorCallback)(char* errMsg);
typedef int (*LogVprintfFptr)(char *fname,char const *fmt, va_list ap);

void FatalErrorfSetCallback(ErrorCallback func);
void ErrorfSetCallback(ErrorCallback func);
void ErrorfPushCallback(ErrorCallback func);
void ErrorfPopCallback(void);

void printWinErr(char* functionName, char* filename, int lineno, int err);
char *lastWinErr();
NORETURN FatalErrorf(FORMAT fmt, ...);
NORETURN FatalErrorFilenamef(FORMAT filename, char const *fmt, ...);
void ErrorfInternal(FORMAT fmt, ...);
void ErrorvInternal(FORMAT fmt, va_list ap);
void ErrorFilenamefInternal(const char *filename, FORMAT fmt, ...);
void ErrorFilenamevInternal(const char *filename, char const *fmt, va_list ap);
void ErrorFilenameDupInternal(const char *filename1, const char *filename2, const char *key, const char *type);

int ErrorfCount(void);
void ErrorfResetCounts(void);

int filelog_vprintf(char *fname, char const *fmt, va_list ap);
int filelog_printf(char *fname, FORMAT fmt, ...);
int log_vprintf(char *fname,char const *fmt, va_list ap);
int log_printf(char *fname, FORMAT fmt, ...);
LogVprintfFptr set_log_vprintf(LogVprintfFptr log_vprintf_fptr);
void verbose_printf(FORMAT fmt, ...);
void errorSetVerboseLevel(int v);
void errorSetTempVerboseLevel(int v);
int errorGetVerboseLevel();

void loadstart_printf(FORMAT fmt, ...);
void loadupdate_printf(FORMAT fmt, ...);
void loadend_printf(FORMAT fmt, ...);

void setShowLoadMemUsage(bool flag);
void setLoadTimingPrecistion(int digits);
int errorWasForceShown(void);



void ErrorfFL(const char * file, int line);
#define Errorf  ErrorfFL(__FILE__, __LINE__),ErrorfInternal
#define Errorv	ErrorfFL(__FILE__, __LINE__),ErrorvInternal
#define ErrorFilenamef  ErrorfFL(__FILE__, __LINE__),ErrorFilenamefInternal
#define ErrorFilenameDup(filename1, filename2, key, type)  ErrorfFL(__FILE__, __LINE__),ErrorFilenameDupInternal(filename1, filename2, key, type)

// these two functions control queueing logged errors to N:\.
// by default, errors are not queued.
void ErrorvBeginQueueing();
void ErrorvEndQueueing();

void errorLogStart(void);
void errorLogFileHasError(const char *file);
void errorLogFileIsBeingReloaded(const char *file);

C_DECLARATIONS_END

#endif

