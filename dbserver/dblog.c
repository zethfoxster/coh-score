#include "dblog.h"
#include "file.h"
#include "utils.h"
#include "comm_backend.h"
#include "timing.h"
#include "error.h"
#include "netcomp.h"
#include "memlog.h"
#include "servercfg.h"

//----------------------------------------------------------------------------------
// in-memory debug log

void dbMemLog(const char* func, const char* s, ...)
{
	va_list ap;
	char buf[MEMLOG_LINE_WIDTH];
	char buf2[MEMLOG_LINE_WIDTH];

	va_start(ap, s);
	vsnprintf(buf,MEMLOG_LINE_WIDTH-1,s,ap);
	buf[MEMLOG_LINE_WIDTH-1] = 0;
	va_end(ap);

	snprintf(buf2,MEMLOG_LINE_WIDTH-1,"%22s : %s",func,buf);
	buf2[MEMLOG_LINE_WIDTH-1] = 0;

	memlog_printf(0, "%s", buf2);
}

// print to the console, strip the thread id stuff
void dbMemLogEcho(const char* s, ...)
{
	va_list ap;
	char buf[MEMLOG_LINE_WIDTH+10];

	va_start(ap, s);
	if (vsprintf(buf,s,ap) < 10) return;
	va_end(ap);

	printf("%s\n", buf);
}


