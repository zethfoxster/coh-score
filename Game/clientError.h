#ifndef CLIENTERROR_H
#define CLIENTERROR_H

void clientErrorfCallback(char* errMsg);
void clientFatalErrorfCallback(char* errMsg);
void clientProductionCrashCallback(char *errMsg);
void PerforceErrorDialog(char* errMsg);

void status_printf(char const *fmt, ...);
void statusLineDraw();

void disableClientCrashReports(void);

#endif
