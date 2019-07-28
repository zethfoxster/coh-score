#ifndef _SVR_INIT_H
#define _SVR_INIT_H


void svrInit(void);
int connectToDbserver(float timeout);
void compactWorkingSet(bool try_logging);

#endif
