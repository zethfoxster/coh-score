#ifndef _THREAD_H
#define _THREAD_H
#include "GlobalAuth.h"

unsigned __stdcall IOThreadServer( void *);
unsigned __stdcall IOThreadAcceptEx( void *);
unsigned __stdcall IOThreadInt( void * );
unsigned __stdcall IOThraedClient2(void *);
unsigned __stdcall ListenThread( void *param );
BOOL CreateIOThread( void  );

extern bool g_bTerminating;
extern int  g_updateKey, g_updateKey2;

#endif